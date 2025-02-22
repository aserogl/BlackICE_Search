from flask import Flask, render_template, request, jsonify
from bs4 import BeautifulSoup
import requests
import src.easy_db as db
import os

app = Flask(__name__)
sites_db = db.DataBase("../../assets/databases/ip_addrs.json")

def is_ip(string: str):
    dot_count = string.count(".")
    if dot_count != 3: return False
    hexlets = string.split('.')
    for hexl in hexlets:
        if not hexl.isalnum():
            return False
    
    return True

def is_online(ip):
    return os.system("fping -c1 -t500 " + ip)

def get_site_text(html_content):
    soup = BeautifulSoup(html_content, "html.parser")
                
    # Удаляем все скрипты и стили
    for script in soup(["script", "style"]):
        script.decompose()
        
    # Получаем текст и очищаем его
    text = soup.get_text()
    lines = (line.strip() for line in text.splitlines())
    text = ' '.join(chunk for chunk in lines if chunk)
    return text

def search_database(
    title_search: str | None = None,
    content_search: str | None = None,
    domain_search: str | None = None,
    ip_search: str | None = None
) -> tuple[list[str], list[str]]:
    
    if title_search == content_search == domain_search == ip_search == None:
        return []

    big_data = sites_db.all()
    
    title_results = set()
    content_results = set()
    domain_results = set()
    ip_results = set()

    all_ips = set()

    for ip, data in big_data.items():
        content = data["content"]
        domain = data["domain"]
        title = data["title"]
        # status_code = data["status_code"]

        if title_search and (title_search in title):
            title_results.add(ip)
            all_ips.add(ip)
        
        if content_search and (content_search in content):
            content_results.add(ip)
            all_ips.add(ip)

        if domain_search and (domain_search in domain):
            domain_results.add(ip)
            all_ips.add(ip)
        
        if ip_search and (ip_search in ip):
            ip_results.add(ip)
            all_ips.add(ip)
    
    to_intersect: list[set] = []
    if not (title_search is None):   to_intersect.append(title_results)
    if not (content_search is None): to_intersect.append(content_results)
    if not (domain_search is None):   to_intersect.append(domain_results)
    if not (ip_search is None):      to_intersect.append(ip_results)

    answer = set()
    curr_set = to_intersect[0]

    if len(to_intersect) == 1:
        answer.union(curr_set)

    if len(to_intersect) == 2:
        answer = curr_set.intersection(
            to_intersect[i+1]
        )

    for i in range(max(0, len(to_intersect) - 2)):
        answer = answer.intersection(
            to_intersect[i+1]
        )
    
    return list(answer), list(all_ips)

def get_link(
    ip_addr: str,
    has_https: bool
):
    if has_https:
        ip_addr = "https://" + ip_addr
    else:
        ip_addr = "http://" + ip_addr

    return ip_addr

def get_content(
    ip_addr: str,
    has_https: bool
):
    ip_addr = get_link(ip_addr, has_https)

    try:
        req = requests.get(ip_addr, verify=False)
        content = ""
        if req.status_code == 200:
            content = req.text
        
        return req.status_code, content
    except:
        return 404, ""

def new_ip_addr(
    ip_addr: str,
    domain:   str,
    has_https: bool
):
    title = ""
    content = ""

    # req = requests.get(ip_addr)
    status, content = get_content(ip_addr, has_https)
    if status == 200:
        if content.count("<title>") != 0:
            title = content[
                content.find("<title>")+len("<title>"):
                content.find("</title>")
            ]

    sites_db.set(
        ip_addr, {
            "content": content,
            "title": title,
            "domain": domain,
            "has-https": has_https
        }
    )

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/submit', methods=['POST'])
def submit():
    data = request.get_json()
    input_value = data.get('input')

    results, all_ips = search_database(
        title_search=input_value,
        domain_search=input_value,
        content_search=input_value,
        ip_search=input_value
    )
    print(f"Search result: {all_ips}")

    return jsonify({
        "results": [{
            "title": sites_db.get(ip)["title"] if len(sites_db.get(ip)["title"]) != 0 else f"[domain] {sites_db.get(ip)["domain"]}", 
            "snippet": get_site_text(sites_db.get(ip)["content"])[:100],
            "url": get_link(ip, sites_db.get(ip)["has-https"])
        } for ip in all_ips]
    })

@app.route('/new_ip', methods=['POST'])
def registrate_new_ip():
    big_data = request.get_data(as_text=True)
    print(big_data)

    for raw_data in big_data.splitlines():
        raw_data = raw_data.split()
        data = {
            "ip": raw_data[0],
            "domain": raw_data[1],
            "has-https": raw_data[2] == 'true'
        }

        if not is_ip(data["ip"]):
            print(f"From {request.remote_addr} ->\n{'-'*10}\n{data["ip"]}\n{'-'*10}\n\n is not IP")
            return "fail"

        new_ip_addr(
            data["ip"],
            data["domain"] if data["domain"] != "__empty" else "",
            data["has-https"]
        )

    return "ok"

if __name__ == '__main__':
    app.run(
        host="192.168.31.100",
        port=8080,
        # debug=True
    )