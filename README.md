------Overview

This project is basically a voice-controlled gadget. You press and hold a small touch sensor, and while you're holding it, a microphone connected to a tiny WiFi board (the ESP32-S3) records what you say. Once you let go, that recording gets sent over WiFi to your computer. On the computer, a program uses an AI model called Whisper to turn your speech into text, then checks if you said a keyword like "turn on" or "turn off." Based on what it hears, it sends a command back to the little board, which then turns an LED on or off.

----------------------------

------Requirements

pip install openai-whisper websockets numpy torch

--------------------------------

------DIAGRAM

<img width="2720" height="3024" alt="voice_assistant_architecture_esp32_whisper_en" src="https://github.com/user-attachments/assets/61e7b746-6e1c-47f8-b11b-a01376cd7ded" />

--------------------------------

------SHEET CONNECTION

<img width="1197" height="773" alt="voice_control" src="https://github.com/user-attachments/assets/55a095fc-5c55-4bf9-85e1-500a333f03e6" />

