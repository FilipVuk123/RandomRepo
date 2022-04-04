import tkinter as tk
from datetime import datetime 


class Timer:
    def __init__(self, font_size = 60, bg_color = "black", font_color = "white"):
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
        self.root.bind('<Return>', self.startstop)
        self.root.mainloop()
        
    def close(self):
        self.root.destroy()

    def make_widgets(self):
        tk.Label(self.root, textvariable=self.sv, font=('ariel 15', self.font_size), pady=200, bg = self.bg_color ,fg= self.font_color).pack()
        
        btn_frame = tk.Frame(self.root)
        btn_frame.pack()
        tk.Button(btn_frame, text='        ', bg = "green", command=self.start).pack()
        tk.Button(btn_frame, text='        ', command=self.stop).pack()
        tk.Button(btn_frame, text='        ', bg = "red", command=self.close).pack()
        
    def start(self):
        if not self.is_running:
            self.start_time = datetime.now()
            self.timer()
            self.is_running = True

    def timer(self):
        self.sv.set(datetime.now() - self.start_time)
        self.after_loop = self.root.after(1, self.timer)

    def stop(self):
        if self.is_running:
            self.root.after_cancel(self.after_loop)
            self.is_running = False

    def startstop(self, event=None):
        if self.is_running:
            self.stop()
        else:
            self.start()

Timer()