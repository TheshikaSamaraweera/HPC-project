from mpi4py import MPI
from concurrent.futures import ThreadPoolExecutor, as_completed
from gemini_api import send_prompt
import time

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

def threaded_prompt_handler(prompts, thread_count=4):
    results = []
    times = []

    def worker(prompt):
        start = time.time()
        try:
            response = send_prompt(prompt)
            end = time.time()
            elapsed = end - start
            return (prompt, response, elapsed, None)
        except Exception as e:
            return (prompt, None, None, str(e))

    with ThreadPoolExecutor(max_workers=thread_count) as executor:
        futures = [executor.submit(worker, prompt) for prompt in prompts]

        for future in as_completed(futures):
            prompt, response, elapsed, error = future.result()
            if error:
                print(f"[Process {rank}]  Prompt: {prompt}")
                print(f"[Process {rank}]    Error: {error}")
            else:
                print(f"[Process {rank}]  Prompt: {prompt}")
                print(f"[Process {rank}]     Execution Time: {elapsed:.3f} sec")
                print(f"[Process {rank}]     Response: {response}\n")
                times.append(elapsed)
                results.append((prompt, response, elapsed))

    return results, times

def load_prompts():
    with open("prompt.txt", "r") as f:
        return [line.strip() for line in f if line.strip()]

def divide_prompts(prompts, rank, size):
    per_process = len(prompts) // size
    remainder = len(prompts) % size
    start = rank * per_process + min(rank, remainder)
    end = start + per_process + (1 if rank < remainder else 0)
    return prompts[start:end]

if __name__ == "__main__":
    if rank == 0:
        print(f"\n Starting hybrid MPI + Threaded execution with {size} processes...\n")

    all_prompts = load_prompts()
    my_prompts = divide_prompts(all_prompts, rank, size)

    print(f"[Process {rank}] Assigned {len(my_prompts)} prompts...\n")

    results, times = threaded_prompt_handler(my_prompts, thread_count=4)

    if times:
        avg_time = sum(times) / len(times)
        print(f"[Process {rank}] Average Execution Time: {avg_time:.3f} sec for {len(times)} successful prompts")
    else:
        print(f"[Process {rank}]  No successful executions to calculate average time.")
