import asyncio
import httpx
import time
import json

API_KEY = ""
URL = f"https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key={API_KEY}"
HEADERS = {"Content-Type": "application/json"}

# Max concurrent requests
MAX_CONCURRENT = 10

semaphore = asyncio.Semaphore(MAX_CONCURRENT)

async def send_prompt(client, prompt, idx):
    payload = {
        "contents": [
            {
                "parts": [{"text": prompt}]
            }
        ]
    }

    start_time = time.time()

    try:
        async with semaphore:
            response = await client.post(URL, headers=HEADERS, json=payload)

        elapsed = time.time() - start_time

        if response.status_code == 200:
            data = response.json()
            result = data['candidates'][0]['content']['parts'][0]['text']
            return (idx, prompt, result, elapsed)
        else:
            return (idx, prompt, f"[HTTP ERROR {response.status_code}] {response.text}", elapsed)

    except Exception as e:
        return (idx, prompt, f"[REQUEST FAILED] {e}", None)

async def main(prompts):
    async with httpx.AsyncClient(timeout=60) as client:
        tasks = [send_prompt(client, prompt, i+1) for i, prompt in enumerate(prompts)]
        results = await asyncio.gather(*tasks)

    total_time = 0
    successful = 0

    print("\n Results:\n")
    for idx, prompt, response, duration in sorted(results):
        print(f"[{idx}] Prompt: {prompt}")
        if duration is not None:
            print(f"    Time: {duration:.2f}s")
            print(f"    Gemini says: {response}\n")
            total_time += duration
            successful += 1
        else:
            print("  Failed:", response, "\n")

    if successful:
        print("Total Prompts:", successful)
        print(f" Total Time: {total_time:.2f}s")
        print(f" Avg Time per Prompt: {total_time/successful:.2f}s")
    else:
        print("No successful responses.")

if __name__ == "__main__":
    with open("prompt.txt", "r") as f:
        prompts = [line.strip() for line in f if line.strip()]

    asyncio.run(main(prompts))
