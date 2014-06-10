self.baseAddr="0x829000"
self.spaceLength="0xd000"
self.vmeregs=[('CODE_ADD', '0x4', ''), ('SERIAL_NUMBER', '0x8', ''), ('VMEVERSION_ADD', '0xc', ''), ('FPGAVERSION_ADD', '0x80', '')]
self.hiddenfuncs=""
self.funcs=[[None, 'address: rel. adress (4, 8, 12,... for 32 bits words readings)\nloops: 0: endless loop\nvalue: 0: read + print\n       1: read only\n      >1: write, no print\nmics: mics between vme reads/writes. 0: do not wait between vme r/w\n', 'void', 'vmeloop', [['address', 'w32', ''], ['loops', 'int', ''], ['value', 'w32', ''], ['mics', 'int', '']]]]
