import asyncio
import signal

MAX_SOCKETS = 5
DELAY = 2

async def connect_socket(i):
    try:
        reader, writer = await asyncio.open_connection("e3r1p2.1337.ma", 6666)
        print("Connected!")
        writer.write("PASS pass\r\n".encode())
        await asyncio.sleep(DELAY)
        writer.write(f"NICK users{i}\r\n".encode())
        await asyncio.sleep(DELAY)
        writer.write(f"USER a{i} 0 * a\r\n".encode())
        await asyncio.sleep(DELAY)
        writer.write("JOIN #general\r\n".encode())
        await asyncio.sleep(DELAY)
        await writer.drain()
        while True:
            data = await reader.read(1024)
            if not data:
                break
            response = data.decode()
            print(f"Received: {response}")

            await asyncio.sleep(DELAY)
            writer.write(f"PRIVMSG #general :A7san Server Fl3alam{i}\r\n".encode())
            await writer.drain()

    except asyncio.CancelledError:
        print(f"Task {i} was cancelled.")
    except Exception as e:
        print(f"Error: {e}")

async def main():
    tasks = []
    for i in range(MAX_SOCKETS):
        task = asyncio.create_task(connect_socket(i))
        tasks.append(task)
    try:
        await asyncio.gather(*tasks)
    except asyncio.CancelledError:
        print("Main task was cancelled.")

def signal_handler(signal, frame):
    print("Ctrl+C pressed. Cleaning up...")
    for task in asyncio.all_tasks():
        task.cancel()

if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    asyncio.run(main())
