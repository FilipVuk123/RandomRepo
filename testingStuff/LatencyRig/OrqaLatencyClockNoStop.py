import tkinter as tk
from datetime import datetime 


class Timer:
    def __init__(self, font_size = 190, bg_color = "black", font_color = "white"):
        try:
            self.root = tk.Tk(className=' ')
            self.font_size = font_size
            self.bg_color = bg_color
            self.font_color = font_color
            self.root.geometry("1280x720")
            self.root.attributes('-fullscreen', True)
            self.root.configure(bg = self.bg_color)
            self.sv = tk.StringVar()
            self.start_time = None
            self.is_running = False

            self.make_widgets()
            self.root.mainloop()
        except KeyboardInterrupt:
            exit()
    def close(self):
        self.root.destroy()

    def make_widgets(self):
        try:
            tk.Label(self.root, textvariable=self.sv, font=('ariel 15', self.font_size), pady=300, bg = self.bg_color ,fg= self.font_color).pack()
        
            btn_frame = tk.Frame(self.root)
            btn_frame.pack()
            self.start()
        
            tk.Button(btn_frame, text='        ', bg = "green", command=self.start).pack()
            tk.Button(btn_frame, text='        ', bg = "red", command=self.close).pack()
        except KeyboardInterrupt:
            self.root.destroy()
            exit()

    def start(self):
        try:
            if not self.is_running:
                self.start_time = datetime.now()
                self.timer()
                self.is_running = True
        except KeyboardInterrupt:
            self.root.destroy()
            exit()
    
    def timer(self):
        try:
            s = str(datetime.now() - self.start_time)[2:-3]
        
            self.sv.set(s)
            self.root.after(1, self.timer)
        except KeyboardInterrupt:
            self.root.destroy()
            exit()
        
Timer()
