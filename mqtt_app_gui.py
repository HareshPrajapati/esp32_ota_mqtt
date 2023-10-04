import tkinter as tk
from tkinter import messagebox
from tkinter import filedialog
from tkinter import ttk
import paho.mqtt.client as mqtt
from tkinter.colorchooser import askcolor
import time
import os


# MQTT Configuration
mqtt_server = "192.168.1.72" #default serever
mqtt_port = 8080             # default port

# MQTT Topics
mqtt_topic_led = "/update/LED/"
mqtt_rgb_topic = "/update/RGB_LED/"
update_start_topic = "/update/start/"
update_write_topic = "/update/write/"
update_stop_topic = "/update/stop/"

# file path
file_to_publish = ""  # Replace with the path to your local file


# flags for OTA
start_update = False
start_done = False
write_done = False
stop_done = False


# gui components
brightness_value = 0
app = 0
switch_text = 0
switch_canvas = 0
fill_oval = 0
switch_canvas_rgb = 0
fill_oval_rgb = 0
ota_button = 0
color_button = 0
brightness_slider = 0


# MQTT Call back
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe("/update/response/")

# MQTT Call back
def on_message(client, userdata, message):
    # print(f"Message {mid} published")
    global start_done, write_done, stop_done
    topic = message.topic
    if topic == "/update/response/":
        payload = message.payload.decode("utf-8")  # Convert the payload to a string
        # print(f"Received message on topic '{topic}': {payload}")
        if payload == "START OK":
            start_done = True
        elif payload == "WRITE OK":
            write_done = True
        elif payload == "STOP OK":
            stop_done = True

# MQTT Client Setup
client = mqtt.Client("SwitchClient")
client.on_connect = on_connect
client.on_message = on_message


# color converter
def rgb_to_number(rgb_value):
    # Remove the "RGB " prefix and split the values
    rgb_str = rgb_value[4:]
    rgb_values = rgb_str.split(",")
    # Convert each RGB component to an integer and create the number
    red = int(rgb_values[0])
    green = int(rgb_values[1])
    blue = int(rgb_values[2])
    # Combine the RGB components into a single number
    rgb_number = (red << 16) + (green << 8) + blue
    return rgb_number

# color converter
def hex_to_rgb(hex_color):
    hex_color = hex_color.lstrip('#')  # Remove the '#' character
    red = int(hex_color[0:2], 16)
    green = int(hex_color[2:4], 16)
    blue = int(hex_color[4:6], 16)
    return red, green, blue




# toggle Switch
def on_switch_toggle():
    global switch_text , switch_canvas , fill_oval ,switch_canvas_rgb , fill_oval_rgb , ota_button , color_button , brightness_slider
    if switch_text.get() == "ON":
        # Switch is turned on, publish "ON" to the topic
        client.publish(mqtt_topic_led, "ON")
        switch_canvas.itemconfig(fill_oval , fill="#00ff00")
        switch_text.set("OFF")
    else:
        client.publish(mqtt_topic_led, "OFF")
        switch_canvas.itemconfig(fill_oval , fill="black")
        switch_text.set("ON")

# pock color for RGB
def on_color_pick():
    global switch_text , switch_canvas , fill_oval ,switch_canvas_rgb , fill_oval_rgb , ota_button , color_button , brightness_slider
    color = askcolor(title="Pick a Color")[1]
    if color:  # Check if a color was selected
        rgb_value = color
        red, green, blue = hex_to_rgb(rgb_value)
        rgb_str = f"RGB {red},{green},{blue}"
        number = rgb_to_number(rgb_str)
        # print('number ',number)
        # Publish the RGB value to the topic
        client.publish(mqtt_rgb_topic, f"RGB {number}")
        switch_canvas_rgb.itemconfig(fill_oval_rgb , fill=rgb_value)

# Change brightess
def on_brightness_change(val):
    global brightness_value
    global switch_text , switch_canvas , fill_oval ,switch_canvas_rgb , fill_oval_rgb , ota_button , color_button , brightness_slider
    brightness_value = int(val)
    # Publish brightness value to the topic
    client.publish(mqtt_topic_led, f"BRIGHTNESS {brightness_value}")
    # Calculate the fill color based on brightness_value
    green_value = int(brightness_value)  # Scale brightness to the 0-255 range
    fill_color = f'#{0:02x}{green_value:02x}00'
    if green_value < 32:
        fill_color = f'#{0:02x}{0:02x}00'
    switch_canvas.itemconfig(fill_oval, fill=fill_color)


# select file for OTA
def open_file_dialog():
    global file_to_publish
    file_to_publish = filedialog.askopenfilename(
        title="Select a .bin Firmware File",
        filetypes=[("Binary Files", "*.bin")],
    )
    if not file_to_publish:
        messagebox.showinfo("No File Selected", "Please select a firmware file for OTA update.")

    _, file_extension = os.path.splitext(file_to_publish)

    if file_extension.lower() != ".bin":
        # The selected file doesn't have a .bin extension
        print("Selected file is not a .bin file. Update canceled.")
        return


# on OTA update events
def on_ota_update():
    global start_update
    global switch_text , switch_canvas , fill_oval ,switch_canvas_rgb , fill_oval_rgb , ota_button , color_button , brightness_slider
    open_file_dialog()
    if not file_to_publish:
        return  # Don't start the update if no file is selected
    start_update = True
    ota_button.config(state="disabled")
    color_button.config(state="disabled")
    brightness_slider.config(state="disabled")
    if start_update:
            publish_file(file_to_publish)
            start_update = False



# OTA process
def publish_file(file_path):
    global start_done, write_done, stop_done
    file_size = os.path.getsize(file_path)
    while not client.is_connected():
        print("Waiting for the MQTT client to connect...")
        time.sleep(1)

    print('Sending start...')
    client.publish(update_start_topic, "START")
    while True:
        if start_done:
            break

    ota_button.config(state="disabled")
    color_button.config(state="disabled")
    brightness_slider.config(state="disabled")

    progress_window = tk.Toplevel(app)
    progress_window.title("OTA Update Progress")
    progress_window.geometry("300x100")

    # Create a progress bar in the pop-up window
    pop_up_progress = ttk.Progressbar(progress_window, orient="horizontal", mode="determinate", length=200)
    pop_up_progress.pack(pady=10)

    def close_popup():
        # Function to close the progress window
        client.publish("/update/cancel/", "CANCEL")
        ota_button.config(state="active")
        color_button.config(state="active")
        brightness_slider.config(state="active")
        progress_window.destroy()
        app.mainloop()

    # Create a Close button in the pop-up window
    close_button = tk.Button(progress_window, text="Cancel", command=close_popup)
    close_button.pack()
    # Read the content of the file
    with open(file_path, "rb") as file:
        percentage_read = 0
        progress_percentage = 0
        while True:
            chunk = bytearray()
            chunk = file.read(4000)
            write_done = False
            percentage_read += len(chunk)
            if not chunk:
                break
            client.publish(update_write_topic, payload=chunk)
            while True:
                if write_done:
                    break
            progress_percentage = (percentage_read / file_size)
            # progress_bar(percentage_read,file_size)
            pop_up_progress["value"] = int(progress_percentage * 100)
            pop_up_progress.update()
            # print('Write [ ',formated_per_val,' % ]')

    print()
    print('Sending stop...')

    client.publish(update_stop_topic, "STOP")
    while True:
        if stop_done:
            break
    print('Update Done')
    ota_button.config(state="active")
    color_button.config(state="active")
    brightness_slider.config(state="active")
    progress_window.destroy()


# on close app
def on_close():
    # Perform any cleanup or other actions before closing the app
    client.publish(mqtt_rgb_topic, f"RGB {0}")
    client.publish(mqtt_topic_led, "OFF")
    client.loop_stop()
    client.disconnect()
    app.quit()


# connect to MQTT
def connect_to_mqtt():
    global mqtt_server, mqtt_port
    mqtt_server = server_entry.get()
    mqtt_port = int(port_entry.get())

    try:
        client.loop_start()
        client.connect(mqtt_server, mqtt_port)
        print(f"Connected to MQTT server at {mqtt_server}:{mqtt_port}")
        server_label.destroy()
        server_entry.destroy()
        port_label.destroy()
        port_entry.destroy()
        connect_button.destroy()
        myGUI()
        messagebox.showinfo("Success", "MQTT connection established successfully!")
    except Exception as e:
        messagebox.showerror("Connection Error", f"Could not connect to {mqtt_server}:{mqtt_port}.\nError: {e}")


# draw GUI after MQTT server connection
def myGUI():
    # Create a Label
    global switch_text , switch_canvas , fill_oval ,switch_canvas_rgb , fill_oval_rgb , ota_button , color_button , brightness_slider
    label = tk.Label(app, text="Toggle the switch to send MQTT message", font=("Helvetica", 14))
    label.pack(pady=10)  # Add some padding

    switch_text = tk.StringVar()
    switch_canvas = tk.Canvas(app, width=40, height=40, bg="#ccc", highlightthickness=0)
    switch_canvas.pack(pady=10)
    fill_oval = switch_canvas.create_oval(5, 5, 35, 35, fill="black")  # Initialize with a green fill
    switch_button = tk.Button(app, textvariable=switch_text, command=on_switch_toggle, font=("Helvetica", 12), bg="#f2f2f2")
    switch_text.set("ON")
    switch_button.pack(pady=10)

    # Brightness Control
    brightness_label = tk.Label(app, text="Brightness Control", font=("Helvetica", 12), bg="#f2f2f2")
    brightness_label.pack(pady=5)
    brightness_slider = tk.Scale(app, from_=0, to=255, orient="horizontal", command=on_brightness_change, troughcolor="green", bg="#f2f2f2")
    brightness_slider.pack()


    switch_canvas_rgb = tk.Canvas(app, width=40, height=40, bg="#ccc", highlightthickness=0)
    switch_canvas_rgb.pack(pady=10)
    fill_oval_rgb = switch_canvas_rgb.create_oval(5, 5, 35, 35, fill="white")  # Initialize with a green fill
    # Color Picker Button
    color_button = tk.Button(app, text="Pick a Color", command=on_color_pick, font=("Helvetica", 12), bg="#4CAF50", fg="white")
    color_button.pack(pady=10)

    # OTA Update Button
    ota_button = tk.Button(app, text="OTA Update", command=on_ota_update, font=("Helvetica", 12), bg="#007BFF", fg="white")
    ota_button.pack(pady=10)



# GUI Setup and app entry point
app = tk.Tk()
app.title("MQTT Switch App")
app.geometry("720x520")  # Set the width and height
app.protocol("WM_DELETE_WINDOW", on_close)
app.configure(bg="#f2f2f2")  # Background color

server_label = tk.Label(app, text="MQTT Server IP:", font=("Helvetica", 12), bg="#f2f2f2")
server_label.pack(pady=5)
server_entry = tk.Entry(app)
server_entry.pack(pady=5)
port_label = tk.Label(app, text="MQTT Port:", font=("Helvetica", 12), bg="#f2f2f2")
port_label.pack(pady=5)
port_entry = tk.Entry(app)
port_entry.pack(pady=5)
connect_button = tk.Button(app, text="Connect to MQTT Server", command=connect_to_mqtt, font=("Helvetica", 12), bg="#007BFF", fg="white")
connect_button.pack(pady=10)

app.mainloop()