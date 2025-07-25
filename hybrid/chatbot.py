# chatbot.py
import requests
import json
import sys

API_KEY = ""  # Replace with actual key
URL = f"https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key={API_KEY}"
headers = {"Content-Type": "application/json"}

def send_prompt(prompt_text):
    data = {"contents": [{"parts": [{"text": prompt_text}]}]}
    response = requests.post(URL, headers=headers, json=data)
    if response.status_code == 200:
        result = response.json()
        print(result['candidates'][0]['content']['parts'][0]['text'])
    else:
        print(f"API Error {response.status_code}: {response.text}")

if __name__ == "__main__":
    send_prompt(sys.argv[1])
