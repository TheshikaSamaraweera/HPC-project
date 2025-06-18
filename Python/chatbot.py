
import google.generativeai as genai


genai.configure(api_key="AIzaSyBHjjC3JFt1PZCBIjYLt6P0_eX9tOWyxzk")


model = genai.GenerativeModel('gemini-1.5-flash')


chat = model.start_chat()

print("Welcome to Gemini Chatbot! Type 'exit' to quit.\n")

while True:
    user_input = input("You: ")

    if user_input.lower() in ["exit", "quit"]:
        print("Chatbot: Goodbye! 👋")
        break

    try:
        response = chat.send_message(user_input)
        print("Chatbot:", response.text)
    except Exception as e:
        print("Error:", e)
