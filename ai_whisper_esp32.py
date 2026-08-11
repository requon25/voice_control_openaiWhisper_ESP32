import asyncio
import numpy as np
import websockets
import whisper

print("Loading Whisper model...")
model = whisper.load_model("base") # Level of the OpenAI Whisper model to be used or downloaded
print("Model ready for transcription.")


async def audio_handler(websocket):
    print("ESP32 connected to server.")
    audio_bytes = bytearray()

    async for message in websocket:
        if isinstance(message, bytes):
            # Accumulate received PCM blocks
            audio_bytes.extend(message)

        elif isinstance(message, str) and message == "END":
            # Validate that there is at least 0.5 seconds of audio (16,000 samples * 2 bytes = 32,000 bytes)
            if len(audio_bytes) < 16000:
                print(
                    "Audio too short or not captured. Try speaking while holding down the sensor."
                )
                audio_bytes = bytearray()  # Reset buffer
                continue

            print("\nProcessing audio in RAM...")

            # 1. Use .copy() to release memory lock from the original buffer
            audio_np = np.frombuffer(audio_bytes, dtype=np.int16).copy()

            # Reset buffer by assigning a new object
            audio_bytes = bytearray()

            # 2. Convert to float32 between -1.0 and 1.0
            audio_float32 = audio_np.astype(np.float32) / 32768.0

            # Optional: Print maximum detected volume for debugging
            max_volume = np.max(np.abs(audio_float32))
            print(f"Maximum audio level received: {max_volume:.4f}")

            if max_volume < 0.01:
                print(
                    "Warning: Audio level is almost zero (silence)."
                )

            # 3. Transcribe
            result = model.transcribe(
                audio_float32, fp16=False, language="en"
            )

            text = result["text"].strip().lower()
            print(f"Final transcription: > {text} <\n")

            # Option customized by users
            if "turn on" in text or "on" in text or "1" in text:
                await websocket.send("LED_ON")
                print("Command sent: Turn ON LED")

            elif "turn off" in text or "off" in text or "0" in text:
                await websocket.send("LED_OFF")
                print("Command sent: Turn OFF LED")


async def main():
    async with websockets.serve(audio_handler, "0.0.0.0", 8765):
        print("WebSocket server listening on ws://0.0.0.0:8765")
        await asyncio.Future()


if __name__ == "__main__":
    asyncio.run(main())
