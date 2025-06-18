# from omp4py import omp
# from gemini_api import send_prompt
# import time
#
# class OMPPromptRunner:
#     def __init__(self, prompts):
#         self.prompts = prompts
#         self.results = [None] * len(prompts)
#         self.times = [None] * len(prompts)
#
#     def run(self):
#         print("Running with OpenMP-style parallelism using OMP4Py")
#
#         @omp
#         def process():
#             for i in range(len(self.prompts)):
#                 prompt = self.prompts[i]
#                 print(f"\nThread {omp.get_thread_num()} handling Prompt {i+1}: {prompt}")
#                 start = time.time()
#                 try:
#                     response = send_prompt(prompt)
#                     self.results[i] = response
#                     elapsed = time.time() - start
#                     self.times[i] = elapsed
#                     print(f"Response for Prompt {i+1}: {response}")
#                     print(f"Time: {elapsed:.2f}s")
#                 except Exception as e:
#                     self.results[i] = f"[ERROR] {e}"
#                     self.times[i] = None
#                     print(f"Error for Prompt {i+1}: {e}")
#
#         process()
#
#         valid_times = [t for t in self.times if t is not None]
#         if valid_times:
#             avg = sum(valid_times) / len(valid_times)
#             print(f"\nAverage Time: {avg:.2f}s")
#         else:
#             print("\nNo successful responses")
#
#         return self.results
#
# def load_prompts(path="prompt.txt"):
#     with open(path, "r", encoding="utf-8") as f:
#         return [line.strip() for line in f if line.strip()]
#
# if __name__ == "__main__":
#     prompts = load_prompts()
#     runner = OMPPromptRunner(prompts)
#     runner.run()
