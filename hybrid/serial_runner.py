import subprocess
import time

PROMPT_FILE = "prompts.txt"
CHATBOT_SCRIPT = "chatbot.py"

def read_prompts(filename):
    with open(filename, "r") as f:
        return [line.strip() for line in f if line.strip()]

def run_prompt(prompt, index):
    print(f"\nPrompt {index:02d}: {prompt}")
    start = time.time()
    result = subprocess.run(
        ["python3", CHATBOT_SCRIPT, prompt],
        capture_output=True,
        text=True
    )
    end = time.time()
    duration = end - start
    print("Response:\n" + result.stdout.strip())
    print(f"Time taken: {duration:.2f} sec")
    return duration

def main():
    prompts = read_prompts(PROMPT_FILE)
    total_time = 0
    times = []

    overall_start = time.time()
    for i, prompt in enumerate(prompts):
        t = run_prompt(prompt, i)
        times.append(t)
        total_time += t
    overall_end = time.time()

    average_time = total_time / len(prompts) if prompts else 0

    print("\n========== SUMMARY ==========")
    print(f"Total time spent (prompt only): {total_time:.2f} sec")
    print(f"Total time (including overhead): {overall_end - overall_start:.2f} sec")
    print(f"Average time per prompt: {average_time:.2f} sec")

if __name__ == "__main__":
    main()
