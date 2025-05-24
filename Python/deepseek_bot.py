from openai import OpenAI

client = OpenAI(
  base_url="https://openrouter.ai/api/v1",
  api_key="",
)

# Optional headers for OpenRouter usage stats (optional but recommended)
extra_headers = {
    "HTTP-Referer": "http://localhost",  # Or your project URL
    "X-Title": "Terminal Chatbot",
}

# Initialize conversation history
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

# Chat loop
print("Chatbot is ready! Type 'exit' to stop.\n")
while True:
    user_input = input("You: ")
    if user_input.lower() in ["exit", "quit"]:
        break

    # Add user message
    messages.append({"role": "user", "content": user_input})

    # Get response from model
    response = client.chat.completions.create(
        model="deepseek/deepseek-r1:free",  # or  deepseek/deepseek-chat
        messages=messages,
        extra_headers=extra_headers
    )

    reply = response.choices[0].message.content
    print("Bot:", reply)

    # Add assistant's reply to the message history
    messages.append({"role": "assistant", "content": reply})