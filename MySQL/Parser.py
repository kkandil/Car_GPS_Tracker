from lxml import html
import requests
import bs4
from selenium import webdriver
import time
from multiprocessing import Pool
import os
from pytube import YouTube


def download_url(url):
    filename = "lucy+li"
    #print("downloading: ",url)
    # assumes that the last segment after the / represents the file name
    # if url is abc/xyz/file.txt, the file name will be file.txt
    file_name_start_pos = url.rfind("/") + 1
    file_name = url[file_name_start_pos:]
    r = requests.get(url, stream=True)
    if r.status_code == requests.codes.ok:
        with open(filename + "\\" + file_name, 'wb') as f:
            for data in r:
                f.write(data)
    return url

def pool_handler(urls):
    p = Pool(10)
    p.map(download_url, urls)


if __name__ == '__main__':

    websit = "https://www.pornpics.com/?q=lucy+li"
    filename = "lucy+li"
    try:
        os.mkdir(filename)
    except:
        pass

    #file_name_start_pos = websit.rfind("=") + 1
    #filename = websit[file_name_start_pos:]
    #filename = str.replace(filename, "+", " ")

    driver = webdriver.PhantomJS(executable_path='C:\\Users\\khale\\Downloads\\Compressed\\phantomjs-2.1.1-windows\\bin\\phantomjs')
    driver.get(websit)

    no_of_pagedowns = 20
    while no_of_pagedowns:
        try:
            # Action scroll down
            driver.execute_script("window.scrollTo(0, document.body.scrollHeight);")
            time.sleep(0.2)
        except:
            pass

        no_of_pagedowns-=1

    # while True:
    #    try:
    #        # Action scroll down
    #        driver.execute_script("window.scrollTo(0, document.body.scrollHeight);")
    #        break
    #    except:
    #        pass

    data = driver.page_source
    driver.close()

    soup = bs4.BeautifulSoup(data, 'lxml')

    links = []
    count = 0
    for li in soup.find_all(class_="thumbwook"):
        count = count + 1
        try:
            link_txt = li.a.get('href')
            if "pornpics" in link_txt:
                links.append(link_txt)
        except:
            pass
    print("# of Gal = " + str(count))
    count = 0
    count2 = 0
    urls = []
    for link in links:
        count = count + 1
        print(count)
        page = requests.get(link)
        tree = html.fromstring(page.content)

        soup = bs4.BeautifulSoup(page.text, 'lxml')


        for li in soup.find_all(class_="thumbwook"):
            imageLink = li.a.get('href')
            urls.append(imageLink)
            count2 = count2 + 1
            #filename = str(count) + ".jpg"
            #f = open(filename, 'wb')
            #f.write(requests.get(imageLink).content)
            #f.close()
            #count = count + 1
        page.close()
        #pool_handler(urls)
    print("# of pics = " + str(count2))
    print("downloading")
    pool_handler(urls)
    print("done")
