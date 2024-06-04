import socket
import datetime
import mysql.connector

SERVER_IP = '192.168.1.117'  # Replace with your Arduino's IP
SERVER_PORT = 7000
INITIAL_MESSAGE = "|OPENEX%"
LOG_FILE = "connection_log.txt"

# MySQL database configuration
db_config = {
    'host': 'localhost',
    'user': 'root',
    'password': '',
    'database': 'paytm_park'
}

def log_message(message):
    with open(LOG_FILE, 'a') as log:
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log.write(f"{timestamp}: {message}\n")

def check_entry_boom():
    try:
        conn = mysql.connector.connect(**db_config)
        cursor = conn.cursor()

        query = "SELECT entryboom FROM boomsig"
        cursor.execute(query)

        result = cursor.fetchone()
        if result:
            entry_boom_status = result[0]
            if entry_boom_status == 'Y':
                log_message("Entry boom is open")
            elif entry_boom_status == 'N':
                log_message("Boom signal is not coming")

        cursor.close()
        conn.close()

    except mysql.connector.Error as err:
        log_message(f"Error: {err}")

def main():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        client_socket.connect((SERVER_IP, SERVER_PORT))
        log_message(f"Connected to {SERVER_IP}:{SERVER_PORT}")

        # Send the initial message upon connection
        client_socket.sendall(INITIAL_MESSAGE.encode())

        while True:
            data = client_socket.recv(1024)
            if data:
                decoded_data = data.decode('utf-8')
                if "|HLT%" in decoded_data:
                    log_message(f"Received health packet: {decoded_data}")
                    check_entry_boom()  # Call check_entry_boom() here to log the status

    except ConnectionRefusedError:
        log_message(f"Connection to {SERVER_IP}:{SERVER_PORT} was refused.")
    except TimeoutError:
        log_message("Connection timed out. Check IP and port.")
    except Exception as e:
        log_message(f"An error occurred: {e}")

    finally:
        client_socket.close()

if __name__ == "__main__":
    main()
