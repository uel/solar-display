import logging

import azure.functions as func

import asyncio
from pyppeteer import launch

from PIL import Image, ImageOps
import numpy as np
import io
import gzip
import base64

from urllib.parse import urlparse

from screens import SCREENS

def IsValidUrl(s):
    try:
        result = urlparse(s)
        return all([result.scheme, result.netloc])
    except:
        return False    

def convert_8bit_to_4bit(img):
    img = img // 17
    fourbit = []
    for i in range(0,len(img),2):
        fourbit.append(img[i] << 4 | img[i+1])
    fourbit = bytes(fourbit)
    return fourbit

# HTML or url
async def GetScreen(s, gray_4bit=False):
    browser = await launch(handleSIGINT=False, handleSIGTERM=False, handleSIGHUP=False)
    page = await browser.newPage()
    await page.setViewport({
        "width": 960,
        "height": 540 });

    if s["type"] == "url":
        await page.goto(s["url"], {"waitUntil": 'networkidle0' })
    elif s["type"] == "html":
        await page.goto(f'data:text/html,{s["html"]}', {"waitUntil": 'networkidle0' })

    if "style" in s:
        await page.addStyleTag(content = s["style"])

    if "element" in s:
        page = await page.querySelector(s["element"])

    img = await page.screenshot(encoding="binary")
    img = Image.open(io.BytesIO(bytes(img)))
    img = ImageOps.pad(img, (960, 540), color='white')
    img = ImageOps.grayscale(img)

    await browser.close()
    
    if gray_4bit:
        img = np.array(img).flatten()
        img = convert_8bit_to_4bit(img)
    else:
        buffered = io.BytesIO()
        img.save(buffered, format="png")
        img = base64.b64encode(buffered.getvalue()).decode()
    
    return img

def main(req: func.HttpRequest) -> func.HttpResponse:
    logging.info('Python HTTP trigger function processed a request.')

    id = req.params.get('id')
    if not id:
        return func.HttpResponse("No screen id")

    try:
        screen = SCREENS[int(id)%len(SCREENS)]()

        if req.params.get('bin'):
            img = asyncio.run(GetScreen(screen, True))
            if req.params.get('gz'): 
                img = gzip.compress(img)
            return func.HttpResponse(img,headers={'content-length':str(len(img))})
        else:
            img = asyncio.run(GetScreen(screen))
            img = f"""<img src="data:image/png;base64, {img}" alt="">"""    
            return func.HttpResponse(status_code=200,headers={'content-type':'text/html'}, body=img)
    except Exception as e:
        return func.HttpResponse(str(e))
