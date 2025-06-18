import time
from gemini_api import send_prompt


def measure_performance(prompts):
    times = []
    results = []

    for i, prompt in enumerate(prompts, start=1):
        print(f"\nPrompt {i}: {prompt}")
        start_time = time.time()
        try:
            response_text = send_prompt(prompt)
            end_time = time.time()
            elapsed = end_time - start_time
            times.append(elapsed)
            results.append(response_text)
            print(f"Execution time: {elapsed:.3f} seconds")
            print("Gemini says:", response_text)
        except Exception as e:
            print("Error:", e)
            times.append(None)
            results.append(None)

    valid_times = [t for t in times if t is not None]
    if valid_times:
        avg_time = sum(valid_times) / len(valid_times)
        print(f"\nAverage execution time for {len(valid_times)} prompts: {avg_time:.3f} seconds")
    else:
        print("\nNo successful requests to calculate average time.")

    return times, results


if __name__ == "__main__":
    with open("prompt.txt", "r") as f:
        prompts = [line.strip() for line in f if line.strip()]

    measure_performance(prompts)
