self.baseAddr="0x400000"
self.spaceLength="0x100"
self.vmeregs=[('ch1_ref', '0x12', '0x04000000'), ('ch2_ref', '0x14', '0x04000000'), ('ch3_ref', '0x16', '0x04000000'), ('ch1_freq_low', '0x18', '0x04000000'), ('ch1_freq_high', '0x1a', '0x04000000'), ('ident_id', '0x8', '0x04000000'), ('card_id', '0x24', '0x04000000'), ('board_id', '0x3a', '0x04000000')]
self.hiddenfuncs=""
self.funcs=[[None, '- set RF board (0x5 for BC and 0x70 for Orbit)\n', 'void', 'writeall', []], [None, 'ch1_ref:0x5\nch2_ref:0x5\nch3_ref:0x70\n', 'void', 'readall', []]]
