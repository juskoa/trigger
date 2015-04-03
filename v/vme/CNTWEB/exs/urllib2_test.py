import urllib2
 
def getPage():
    url="http://pcalicebhm10"
 
    req = urllib2.Request(url)
    response = urllib2.urlopen(req)
    return response.read()
 
if __name__ == "__main__":
    namesPage = getPage()
    print namesPage
