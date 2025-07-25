from mpi4py import MPI
import time
from gemini_api import send_prompt

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

def split_data(data, size):
    """Split data evenly across size workers."""
    avg = len(data) // size
    remainder = len(data) % size
    chunks = []
    start = 0
    for i in range(size):
        end = start + avg + (1 if i < remainder else 0)
        chunks.append(data[start:end])
        start = end
    return chunks

def process_prompts(prompts):
    results = []
    times = []
    for i, prompt in enumerate(prompts, start=1):
        print(f"[Rank {rank}] Prompt {i}: {prompt}")
        start_time = time.time()
        try:
            response = send_prompt(prompt)
            elapsed = time.time() - start_time
            results.append((prompt, response, elapsed))
            print(f"[Rank {rank}]  Response: {response} | Time: {elapsed:.2f}s")
        except Exception as e:
            results.append((prompt, f"[ERROR] {e}", None))
            print(f"[Rank {rank}] Error: {e}")
    return results

if __name__ == "__main__":
    if rank == 0:
        with open("prompt.txt", "r") as f:
            prompts = [line.strip() for line in f if line.strip()]
        prompt_chunks = split_data(prompts, size)
    else:
        prompt_chunks = None

    # sends each process its respective chunk of prompts.
    local_prompts = comm.scatter(prompt_chunks, root=0)

    # Each process handles its local prompts
    local_results = process_prompts(local_prompts)

    # Gather results back to the root process
    all_results = comm.gather(local_results, root=0)

    if rank == 0:
        flat_results = [item for sublist in all_results for item in sublist]
        total_time = sum(res[2] for res in flat_results if res[2] is not None)
        count = sum(1 for res in flat_results if res[2] is not None)
        avg_time = total_time / count if count > 0 else 0

        print("\n All Results:")
        for i, (prompt, response, t) in enumerate(flat_results, 1):
            print(f"\n{i}. Prompt: {prompt}")
            print(f"   Response: {response}")
            print(f"   Time: {t:.2f}s" if t is not None else "   Time: [ERROR]")

        print(f"\nTotal processed: {len(flat_results)}")
        print(f"  Total execution time (sum of individual calls): {total_time:.2f}s")
        print(f" Average response time per prompt: {avg_time:.2f}s")

