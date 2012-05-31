class Kanvas(Canvas):
  def __init__(self, tlw, canvasDestroyed,**kw):
    selfargs=(self,tlw)
    apply(Canvas.__init__,selfargs, kw, width=canvasw,height=canvash,
      background='yellow', borderwidth=1)
    self.canvas.pack(expand=1,fill=BOTH, side=BOTTOM)
    self.canvas.bind("<Destroy>",canvasDestroyed)


