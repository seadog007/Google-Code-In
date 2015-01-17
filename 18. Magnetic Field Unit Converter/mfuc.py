#!/usr/bin/python
# -*- coding: utf-8 -*-

try:
    # for Python2
    from Tkinter import *
except ImportError:
    # for Python3
    from tkinter import *

def factor(unit):
    return {
        u'Gamma (γ)': 1000000000,
        u'Gauss (G)': 10000,
        u'Kilotesla (kT)': 0.001,
        u'Line per square centimetre': 10000, 
        u'Maxwell per square centimetre (Mw/cm²)': 10000, 
        u'Megatesla': 1.0E-6,
        u'Microtesla (µT)': 1000000, 
        u'Millitesla (mT)': 1000, 
        u'Nanotesla (nT)': 1000000000, 
        u'Picotesla (pT)': 1000000000000, 
        u'Tesla (T)': 1, 
        u'Weber per square metre (Wb/m²)': 1
    }[unit]

class simpleapp_tk(Tk):
    def __init__(self, parent):
        Tk.__init__(self, parent)
        self.parent = parent
        self.initialize()

    def initialize(self):
        self.grid()

        self.leftvaule = StringVar()
        self.entry = Entry(self, textvariable=self.leftvaule)
        self.entry.grid(column=0, row=0, sticky='EW')
        self.entry.bind("<Return>", self.OnPressEnter)

        self.leftunit = StringVar()
        self.leftunit.set("Gamma (γ)")
        optMenu = OptionMenu(self, self.leftunit, "Gamma (γ)", "Gauss (G)", "Kilotesla (kT)", "Line per square centimetre", "Maxwell per square centimetre (Mw/cm²)", "Megatesla", "Microtesla (µT)", "Millitesla (mT)", "Nanotesla (nT)", "Picotesla (pT)", "Tesla (T)", "Weber per square metre (Wb/m²)")
        optMenu.grid(column=1, row=0)

        label = Label(self, text=" = ")
        label.grid(column=2, row=0, sticky='EW')

        self.rightvaule = StringVar()
        self.entry = Label(self, textvariable=self.rightvaule)
        self.entry.grid(column=3, row=0, sticky='EW')
        self.rightvaule.set("Res")

        self.rightunit = StringVar()
        self.rightunit.set("Gamma (γ)")
        optMenu = OptionMenu(self, self.rightunit, "Gamma (γ)", "Gauss (G)", "Kilotesla (kT)", "Line per square centimetre", "Maxwell per square centimetre (Mw/cm²)", "Megatesla", "Microtesla (µT)", "Millitesla (mT)", "Nanotesla (nT)", "Picotesla (pT)", "Tesla (T)", "Weber per square metre (Wb/m²)")
        optMenu.grid(column=4, row=0)

        button = Button(self, text=u"Convert", command=self.OnButtonClick)
        button.grid(column=5, row=0)

        self.grid_columnconfigure(0, weight=1)
        self.resizable(True, True)

    def OnButtonClick(self):
        self.rightvaule.set(float(factor(self.rightunit.get()))/float(factor(self.leftunit.get()))*float(self.leftvaule.get()))

    def OnPressEnter(self, event):
        self.rightvaule.set(float(factor(self.rightunit.get()))/float(factor(self.leftunit.get()))*float(self.leftvaule.get()))

if __name__ == "__main__":
    app = simpleapp_tk(None)
    app.title('Magnetic Field Unit Converter')
    app.mainloop()
