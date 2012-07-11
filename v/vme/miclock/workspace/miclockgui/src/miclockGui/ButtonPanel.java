package miclockGui;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.IOException;

import javax.swing.ButtonGroup;
import javax.swing.ButtonModel;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JToggleButton;

/**
 * A panel containing all the buttons used in the main frame of this program.
 * @author oerjan
 *
 */
public class ButtonPanel extends JPanel 
	implements ActionListener, PropertyChangeListener
{
	private JLabel operLabel;
	private JLabel clockLable;
	
	private JToggleButton autoButton;
	private JToggleButton manuButton;
	
	private JToggleButton beam1Button;
	private JToggleButton beam2Button;
	private JToggleButton refButton;
	private JToggleButton localButton;
	
	private JButton resetShiftButton;
	private JButton getShiftButton;
	
	private ButtonGroup	amGroup;
	private ButtonGroup bbrlGroup;
	
	private MiClockClient client;
//	private ClockShiftPanel csPanel;
	
	private GridBagConstraints constraints;
	
	// used to make sure that there is not added duplicate buttons
	// from the actionPerformed() method.
	private boolean auto = true;
	
	// used to disable buttons while there is a clock transition under way
	private boolean intrans;
	
	// used to store the state which is being transitioned to
	private String transTo;
	
	// used to store the shift. Needed for the shift dialog.
	private String shift;
	
	/**
	 * Constructor
	 * @param client The MiClockClient for this program.
	 */
	public ButtonPanel(MiClockClient client)
	{
		this.client = client;
		this.client.addPropertyChangeListener(this);
		
		this.setLayout(new GridBagLayout());
		this.constraints = new GridBagConstraints();
		
		this.constraints.gridx = 0;
		this.constraints.gridy = 0;
		this.constraints.gridheight = 1;
		this.constraints.gridwidth = 1;
		this.constraints.fill = GridBagConstraints.BOTH;
		this.constraints.weightx = 0.5;
		this.constraints.weighty = 0.5;
		
		this.operLabel = new JLabel("Operation:");
		
		this.add(this.operLabel, this.constraints);
		
		
		this.autoButton = new JToggleButton("Auto");
		this.manuButton = new JToggleButton("Manual");
		
		this.autoButton.setName("auto");
		this.manuButton.setName("manu");
		
		this.autoButton.addActionListener(this);
		this.manuButton.addActionListener(this);
		
		this.autoButton.setMnemonic(KeyEvent.VK_A);
		this.manuButton.setMnemonic(KeyEvent.VK_M);
		
		this.amGroup = new ButtonGroup();
		this.amGroup.add(this.autoButton);
		this.amGroup.add(this.manuButton);
		
		this.constraints.gridwidth = 2;
		this.constraints.gridx = 1;
		this.add(this.autoButton, this.constraints);
		this.constraints.gridx = 3;
		this.add(this.manuButton, this.constraints);
		
		this.clockLable = new JLabel("Set Clock:");
		
		this.beam1Button = new JToggleButton("Set Beam1");
		this.localButton = new JToggleButton("Set Local");
		
		this.beam1Button.setName("beam1");
		this.localButton.setName("local");
		
		this.beam1Button.addActionListener(this);
		this.localButton.addActionListener(this);
		
		this.bbrlGroup = new ButtonGroup();
		this.bbrlGroup.add(this.beam1Button);
		this.bbrlGroup.add(this.localButton);

		this.constraints.gridx = 0;
		this.constraints.gridy = 1;
		this.constraints.gridwidth = 1;
		
		this.add(this.clockLable, this.constraints);
		
		this.constraints.gridx = 1;
		this.add(this.beam1Button, this.constraints);
		this.constraints.gridx = 4;
		this.add(this.localButton, this.constraints);
		
		this.beam1Button.setMnemonic(KeyEvent.VK_1);
		this.localButton.setMnemonic(KeyEvent.VK_L);
		
		if(Main.ENABLE_EXTRA_BUTTONS)
		{
			this.beam2Button = new JToggleButton("Set Beam2");
			this.refButton = new JToggleButton("Set Ref");
			
			this.beam2Button.setName("beam2");
			this.refButton.setName("ref");
			
			this.beam2Button.addActionListener(this);
			this.refButton.addActionListener(this);
			
			this.bbrlGroup.add(this.beam2Button);
			this.bbrlGroup.add(this.refButton);
			
			this.constraints.gridx = 2;
			this.add(this.beam2Button, this.constraints);
			this.constraints.gridx = 3;
			this.add(this.refButton, this.constraints);	
			
			this.beam2Button.setMnemonic(KeyEvent.VK_2);
			this.refButton.setMnemonic(KeyEvent.VK_R);
		}
		
		this.resetShiftButton = new JButton("Reset Shift");
		this.getShiftButton = new JButton("Get Shift");
		
		this.resetShiftButton.setName("shift");
		this.getShiftButton.setName("getShift");
		
		this.resetShiftButton.addActionListener(this);
		this.getShiftButton.addActionListener(this);
		
		this.resetShiftButton.setMnemonic(KeyEvent.VK_S);
		this.resetShiftButton.setMnemonic(KeyEvent.VK_G);
		
		this.constraints.gridx = 1;
		this.constraints.gridy = 3;
		
		this.add(this.resetShiftButton, this.constraints);
		this.constraints.gridx = 2;
		this.add(this.getShiftButton, this.constraints);
		
		/*
		csPanel = new ClockShiftPanel(client);
		constraints.gridy = 2;
		constraints.gridx = 0;
		constraints.gridwidth = 0;
		this.add(csPanel, constraints);
		*/
		this.manuButton.doClick();
	}
	
	/**
	 * Enables/Disables the BEAM1, BEAM2, REF and LOCAL buttons.
	 * @param b Boolean to enable/disable the buttons
	 */
	private void setEnableClockSelect(boolean b)
	{
		this.beam1Button.setEnabled(b);
		this.localButton.setEnabled(b);
		this.bbrlGroup.clearSelection();
		
		if(Main.ENABLE_EXTRA_BUTTONS)
		{
			this.beam2Button.setEnabled(b);
			this.refButton.setEnabled(b);			
		}
		if(b)
		{
			if(this.client.getMiclock().equals("BEAM1"))
			{
				this.bbrlGroup.setSelected(this.beam1Button.getModel(), true);
			}
			else if(this.client.getMiclock().equals("BEAM2"))
			{
				this.bbrlGroup.setSelected(this.beam2Button.getModel(), true);
			}
			else if(this.client.getMiclock().equals("REF"))
			{
				this.bbrlGroup.setSelected(this.refButton.getModel(), true);
			}
			else if(this.client.getMiclock().equals("LOCAL"))
			{
				this.bbrlGroup.setSelected(this.localButton.getModel(), true);
			}
		}
	}
	
	// TODO: Better method name
	/**
	 * Method for determening the clock state to be in when in Auto mode.
	 * The value for this is set in Main.AUTO_BEAM_MIN and Main.AUTO_BEAM_MAX.
	 * 4-11 -> BEAM1 (10.07.12)
	 * 1-3,12-21 -> LOCAL (10.07.12)
	 * @param state Integer of which mode the beam is in.
	 */
	private void autoMode(int state)
	{
		// TODO : Shift to be done automatic in SQUEEZE
		if((state >= Main.AUTO_BEAM_MIN && state <= Main.AUTO_BEAM_MAX) 
				&& this.client.getMiclock().equals("LOCAL") )
		{
			this.client.sendMiClockSetCommand("BEAM1");
		}
		else if((state < Main.AUTO_BEAM_MIN || state > Main.AUTO_BEAM_MIN
				) 
				&& this.client.getMiclock().equals("BEAM1"))
		{
			this.client.sendMiClockSetCommand("LOCAL");
		}
		
		if(state == 9 && !this.client.getMiclock().equals("LOCAL"))
		{
			this.client.sendGetShift();
			if(!shift.equals("old"))
			{
				Float floatShift = Float.parseFloat(shift);
				System.out.println(floatShift);
				int value = (int) Math.round((floatShift*100*(-1)));
				
				// TODO: Test this
				/*
				if(value >= 1500)
				{
					Runtime rt = Runtime.getRuntime();
					try {
						Process pro = rt.exec("cat /dev/null | /opt/infoLogger/log -l OPS -s f Clockshift too big");
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
				
				else
				{
				*/
					System.out.println(value);
					client.sendShiftCommand(value);
//				}
			}
		}
	}
	
	// TODO: Better method name
	/**
	 * Enables/Disables the right buttons when switching from auto to manual
	 * mode and is in a clock transition.
	 */
	private void manuMode()
	{
		if(!this.auto && this.intrans)
		{
			ButtonModel bm = null;
			if(this.transTo.equals("BEAM1"))
			{
				bm = this.beam1Button.getModel();
			}
			else if(this.transTo.equals("BEAM2"))
			{
				bm = this.beam2Button.getModel();
			}
			else if(this.transTo.equals("REF"))
			{
				bm = this.refButton.getModel();
			}
			else if(this.transTo.equals("LOCAL"))
			{
				bm = this.localButton.getModel();
			}
			this.setEnableClockSelect(false);
			bm.setEnabled(true);
			this.bbrlGroup.setSelected(bm, true);
		}
		else if(!this.auto && !this.intrans)
		{
			this.setEnableClockSelect(true);
		}
		
	}

	/**
	 * Enables/Disables the shift button in the right states.
	 * The value of when to enable/disable is set in Main.SHIFT_VALID_MIN and
	 * Main.SHIFT_VALID_MAX.
	 * 8-11 -> Enable (10.07.12)
	 * 1-7,12-21 -> Disable (10.07.12)
	 * @param state Beam mode state
	 */
	private void setEnableShift(int state)
	{
		// TODO: Remove Magic numbers
		if(state >= Main.SHIFT_VALID_MIN && state <= Main.SHIFT_VALID_MAX)
		{
			this.resetShiftButton.setEnabled(true);
		}
		else
		{
			this.resetShiftButton.setEnabled(false);
		}
	}
	
	/**
	 * Handles what to do when a Property Change Event is fired.
	 * Part of the PropertyChangeListener interface.
	 * @param evt 
	 */
	public void propertyChange(PropertyChangeEvent evt) {
		if(evt.getPropertyName().equals("beammode"))
		{
			if(this.auto && !this.intrans)
			{
				this.autoMode(this.client.getBeammodeInt());
			}
			
			//TODO: Find getfsdip.py
			/*
			if(this.client.getBeammodeInt() == 6)
			{
				Runtime rt = Runtime.getRuntime();
				try {
					Process pro = rt.exec("");
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
			*/
			this.setEnableShift(this.client.getBeammodeInt());
			
		}
		else if(evt.getPropertyName().equals("clocktrans"))
		{
			int clock = Integer.parseInt(this.client.getClocktrans());
			if(clock == 0)
			{
				this.intrans = false;
			}
			else if(clock >= 1)
			{
				this.intrans = true;
			}
			this.manuMode();
		}
		else if(evt.getPropertyName().equals("sendClock"))
		{
			this.transTo = (String)evt.getNewValue();
		}
		
		else if(evt.getPropertyName().equals("getShift"))
		{
			this.shift = (String) evt.getNewValue();
		}
	}		
	
	/**
	 * Handles what do to when a ActionEvent is fired.
	 * Part of the ActionListener interface.
	 * @param evt
	 */
	public void actionPerformed(ActionEvent evt) {
		if(!Main.GUI_SHELL_MODE)
		{
			if(evt.getSource() == this.autoButton)
			{
				this.setEnableClockSelect(false);
				this.auto = true;
				this.client.sendOperChange("AUTO");
				this.autoMode(this.client.getBeammodeInt());
			}
			else if (evt.getSource() == this.manuButton)
			{
				this.setEnableClockSelect(true);
				this.auto = false;
				this.client.sendOperChange("MANUAL");
				this.manuMode();
			}
			else if (evt.getSource() == this.beam1Button)
			{
				this.client.sendMiClockSetCommand("BEAM1");
			}
			else if(evt.getSource() == this.beam2Button)
			{
				this.client.sendMiClockSetCommand("BEAM2");
			}
			else if(evt.getSource() == this.refButton)
			{
				this.client.sendMiClockSetCommand("REF");
			}
			else if(evt.getSource() == this.localButton)
			{
				this.client.sendMiClockSetCommand("LOCAL");
			}
			else if(evt.getSource() == this.resetShiftButton)
			{
				this.client.sendGetShift();
				int res = JOptionPane.showConfirmDialog(this, "Current clock" +
						" shift is: "+this.shift+
						"\nReset the clock?"
						, "Clock Shift", JOptionPane.OK_CANCEL_OPTION);
				if(res == JOptionPane.OK_OPTION)
				{
					if(!this.shift.equals("old"))
					{
						Float temp = Float.parseFloat(shift);
						System.out.println(temp);
						int value = (int) Math.round((temp*100*(-1)));
						System.out.println(value);
						client.sendShiftCommand(value);
					}
				}
			}
			else if(evt.getSource() == this.getShiftButton)
			{
				this.client.sendGetShift();
			}
		}
	}
}

