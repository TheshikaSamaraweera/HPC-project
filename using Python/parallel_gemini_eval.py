import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from gemini_api import send_prompt  # uses your existing send_prompt() function

def process_prompt(i, prompt):
    start = time.time()
    try:
        response = send_prompt(prompt)
        duration = time.time() - start
        return (i, prompt, response, duration)
    except Exception as e:
        return (i, prompt, f"[ERROR] {e}", None)

def parallel_evaluation(prompts, max_workers=4):
    print(f"\n🔄 Running in parallel with {max_workers} threads...\n")
    results = []
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = [executor.submit(process_prompt, i+1, p) for i, p in enumerate(prompts)]

        for future in as_completed(futures):
            i, prompt, response, duration = future.result()
            if duration is not None:
                print(f"[{i}] Prompt: {prompt}")
                print(f"    Response Time: {duration:.2f}s")
                print(f"    Gemini says: {response}\n")
            else:
                print(f"[{i}] Prompt: {prompt} ❌ Failed")
                print(f"    Error: {response}\n")

            results.append((prompt, response, duration))

    durations = [d for _, _, d in results if d is not None]
    if durations:
        avg_time = sum(durations) / len(durations)
        print(f"\n✅ Average Response Time: {avg_time:.2f}s")
        print(f"⚡ Total Time (Parallel): {sum(durations):.2f}s")
    else:
        print("\nNo successful responses.")

    return results


if __name__ == "__main__":
    with open("prompt.txt", "r") as f:
        prompts = [line.strip() for line in f if line.strip()]

    parallel_evaluation(prompts, max_workers=8)
