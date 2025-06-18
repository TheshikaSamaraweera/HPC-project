import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from gemini_api import send_prompt

class ParallelPromptRunner:
    def __init__(self, prompts, max_workers=5, batch_size=4, retry_limit=3, delay=0, batch_delay=0):
        self.prompts = prompts
        self.max_workers = max_workers
        self.batch_size = batch_size
        self.retry_limit = retry_limit
        self.delay = delay
        self.batch_delay = batch_delay

    def send_prompt_with_retry(self, prompt):
        for attempt in range(self.retry_limit):
            try:
                response = send_prompt(prompt)
                return prompt, response
            except Exception as e:
                time.sleep(self.delay)
                if attempt == self.retry_limit - 1:
                    return prompt, f"[ERROR] {e}"

    def chunk(self, data, size):
        return [data[i:i+size] for i in range(0, len(data), size)]

    def run(self):
        print("Running in PARALLEL mode")
        all_results = []
        chunks = self.chunk(self.prompts, self.batch_size)
        total_start_time = time.time()

        for i, batch in enumerate(chunks, start=1):
            print(f"\nProcessing Batch {i} (Size: {len(batch)})")
            batch_start = time.time()

            with ThreadPoolExecutor(max_workers=self.max_workers) as executor:
                futures = [executor.submit(self.send_prompt_with_retry, prompt) for prompt in batch]
                for future in as_completed(futures):
                    prompt, result = future.result()
                    print(f"\n Prompt: {prompt}")
                    print(f"Gemini says: {result}")
                    all_results.append((prompt, result))

            print(f" Batch {i} completed in {time.time() - batch_start:.2f}s")

            if i < len(chunks):
                print(f"Waiting {self.batch_delay}s before next batch...")
                time.sleep(self.batch_delay)

        total_time = time.time() - total_start_time
        avg_time = total_time / len(all_results) if all_results else 0

        print(f"\nFinished all batches. Total prompts: {len(all_results)}")
        print(f" Total execution time: {total_time:.2f}s")
        print(f" Average time per prompt: {avg_time:.2f}s")

        return all_results


def load_prompts(path="prompt.txt"):
    with open(path, "r", encoding="utf-8") as f:
        return [line.strip() for line in f if line.strip()]

if __name__ == "__main__":
    prompts = load_prompts()
    runner = ParallelPromptRunner(prompts)
    runner.run()
