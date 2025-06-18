import time
from openai import OpenAI

# Initialize OpenAI client
client = OpenAI(
    base_url="https://openrouter.ai/api/v1",
    api_key="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",
)

# Optional headers
extra_headers = {
    "HTTP-Referer": "http://localhost",
    "X-Title": "Terminal Chatbot",
}

# System message
messages = [
    {
        "role": "system",
        "content": (
            "You are SmartBot, a fast and intelligent AI assistant. "
            "Always give accurate, direct, and well-structured answers. "
            "Avoid unnecessary words. If the answer requires steps, list them clearly. "
            "Keep it concise and insightful. Use markdown formatting when needed."
        )
    }
]

# Initialize metrics
metrics = {
    "total_prompts": 0,
    "total_time": 0.0,
}

print("🤖 Chatbot is ready! Type 'exit' to stop.\n")

# Chat loop
while True:
    user_input = input("You: ")
    if user_input.lower() in ["exit", "quit"]:
        break

    messages.append({"role": "user", "content": user_input})

    # Start timer
    start = time.time()

    # API call
    response = client.chat.completions.create(
        model="deepseek/deepseek-r1:free",
        messages=messages,
        extra_headers=extra_headers
    )

    # End timer and calculate elapsed time
    elapsed = time.time() - start

    reply = response.choices[0].message.content
    print("🤖 Bot:", reply)
    print(f"⏱️ Time: {elapsed:.3f} seconds\n")

    # Add reply to history
    messages.append({"role": "assistant", "content": reply})

    # Update metrics
    metrics["total_prompts"] += 1
    metrics["total_time"] += elapsed

# Summary after chat ends
print("\n📊 Chatbot Performance Summary 📊")
print(f"Total Prompts: {metrics['total_prompts']}")
print(f"Total Time: {metrics['total_time']:.3f} seconds")
if metrics["total_prompts"] > 0:
    avg_time = metrics["total_time"] / metrics["total_prompts"]
    print(f"Average Time per Prompt: {avg_time:.3f} seconds")
print()
