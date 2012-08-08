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
	private JLabel shiftLabel;
	
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
	
	private GridBagConstraints constraints;
	
	// used to make sure that there is not added duplicate buttons
	// from the actionPerformed() method.
	private boolean auto = true;
	
	// used to disable buttons while there is a clock transition under way
	private boolean intrans;
	
	// used to store the state which is being transitioned to
	private String transTo = "";
	
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
		this.constraints.weightx = 1.0;
		this.constraints.weighty = 0.5;
		
		this.operLabel = new JLabel("Operation:");
		
		this.add(this.operLabel, this.constraints);
		this.constraints.weightx = 8.5;
		
		this.autoButton = new JToggleButton("Auto");
		this.manuButton = new JToggleButton("Manual");
		
		this.autoButton.setName("auto");
		this.manuButton.setName("manu");
		
		this.autoButton.setToolTipText("Sets the program in Automatic mode "
				+ "where clock transitions are automatic");
		this.manuButton.setToolTipText("Sets the program in Manual mode where "
				+ "the user controls the clock transitions");
		
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
		
		this.beam1Button.setToolTipText("Transitions the clock to BEAM1");
		this.localButton.setToolTipText("Transitions the clock to LOCAL");
		
		this.beam1Button.addActionListener(this);
		this.localButton.addActionListener(this);
		
		this.bbrlGroup = new ButtonGroup();
		this.bbrlGroup.add(this.beam1Button);
		this.bbrlGroup.add(this.localButton);

		this.constraints.gridx = 0;
		this.constraints.gridy = 1;
		this.constraints.gridwidth = 1;
		
		this.constraints.weightx = 1.0;
		this.add(this.clockLable, this.constraints);
		this.constraints.weightx = 8.5;
		
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
			
			this.beam2Button.setToolTipText("Transitions the clock to BEAM2");
			this.refButton.setToolTipText("Transitions the clock to REF");
			
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
		
		this.shiftLabel = new JLabel("Shift:");
		
		this.resetShiftButton = new JButton("Reset Shift");
		this.getShiftButton = new JButton("Get Shift");
		
		this.resetShiftButton.setName("shift");
		this.getShiftButton.setName("getShift");
		
		this.resetShiftButton.setToolTipText("Gets the newest shift value and "
				+ "opens a dialog to confirm the resetting of the shift");
		this.getShiftButton.setToolTipText("Gets the newest shift value");
		
		this.resetShiftButton.addActionListener(this);
		this.getShiftButton.addActionListener(this);
		
		this.resetShiftButton.setMnemonic(KeyEvent.VK_S);
		this.getShiftButton.setMnemonic(KeyEvent.VK_G);
		
		this.constraints.gridx = 0;
		this.constraints.gridy = 3;
		this.constraints.weightx = 1.0;
		this.add(this.shiftLabel, this.constraints);
		this.constraints.weightx = 8.5;
		
		this.constraints.gridx = 1;
		
		this.add(this.resetShiftButton, this.constraints);
		if(Main.ENABLE_EXTRA_BUTTONS)
		{
			this.constraints.gridx = 2;	
		}
		else
		{
			this.constraints.gridx = 4;
		}
		this.add(this.getShiftButton, this.constraints);
		
		this.manuButton.doClick();
	}
	
	/**
	 * Enables/Disables the BEAM1, BEAM2, REF and LOCAL buttons.
	 * @param b Boolean to enable/disable the buttons
	 */
	private void setEnableButtons(boolean b)
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
	
	
	/**
	 * Method for determening the clock state to be in when in Auto mode.
	 * The value for this is set in Main.AUTO_BEAM_MIN and Main.AUTO_BEAM_MAX.
	 * 4-11 -> BEAM1 
	 * 1-3,12-21 -> LOCAL 
	 * Performs automatic clock shift in SQUEEZ.
	 * @param state Integer of which mode the beam is in.
	 */
	private void autoMode(int state)
	{
		if((state >= Main.AUTO_BEAM_MIN && state < Main.AUTO_BEAM_MAX) 
				&& this.client.getMiclock().equals("LOCAL") )
		{
			this.client.sendMiClockSetCommand("BEAM1");
		}
		else if((state < Main.AUTO_BEAM_MIN || state > Main.AUTO_BEAM_MAX
				) 
				&& this.client.getMiclock().equals("BEAM1"))
		{
			this.client.sendMiClockSetCommand("LOCAL");
		}
		
		
		if(state == 9 && !this.client.getMiclock().equals("LOCAL"))
		{
			this.client.sendGetShift();
			if(!this.shift.equals("old"))
			{
				Float floatShift = Float.parseFloat(this.shift);
				int value = Math.round((floatShift*100*(-1)));
				
				// TODO: Test this
				if(Math.abs(value) >= Main.SHIFT_UPPER_LIMIT)
				{
					this.client.execOptInfoLogger();
				}
				
				else if(Math.abs(value) >= Main.SHIFT_LOWER_LIMIT)
				{
					this.client.sendShiftCommand(value);
				}
				else
				{
					this.client.sendShiftTooBigSmall("small");
				}
			}
		}
	}
	
	/**
	 * Enables/Disables the correct buttons when switching from auto to manual
	 * mode and is in a clock transition.
	 */
	private void transButtonCtrl()
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
			this.setEnableButtons(false);
			bm.setEnabled(true);
			this.bbrlGroup.setSelected(bm, true);
		}
		else if(!this.auto && !this.intrans)
		{
			this.setEnableButtons(true);
		}
		
		
	}

	/**
	 * Enables/Disables the shift button in the correct states.
	 * The value of when to enable/disable is set in Main.SHIFT_VALID_MIN and
	 * Main.SHIFT_VALID_MAX.
	 * 8-11 -> Enable 
	 * 1-7,12-21 -> Disable 
	 * @param state Beam mode state
	 */
	private void setEnableShift(int state)
	{
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
	 * Method for reseting the shift manualy. Gets the shift, creates a dialog
	 * where the operator can confirm the shifting of the clock. If shift is
	 * old/too small message dialog is given.
	 */
	private void resetShift()
	{
		this.client.sendGetShift();
		if(!this.shift.equals("old"))
		{
			Float temp = Float.parseFloat(this.shift);
			int value = Math.round((temp*100*(-1)));
			if(Math.abs(value) < Main.SHIFT_LOWER_LIMIT)
			{
				JOptionPane.showMessageDialog(this, "Shift too small to" +
						" reset");
			}
			else
			{
				int res = JOptionPane.showConfirmDialog(this, "Current " +
						"clock shift is: "+this.shift+ "\nReset the clock?"
						, "Clock Shift", JOptionPane.OK_CANCEL_OPTION);
				if(res == JOptionPane.OK_OPTION)
				{
					this.client.sendShiftCommand(value);
				}
			}
		}
		else
		{
			JOptionPane.showMessageDialog(this, "Shift too old to" +
			" reset");
		}
	}
	
	/**
	 * Handels what to do when a Property Change Event is fired.
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
			if(this.client.getBeammodeInt() == 9 && 
					!this.client.getMiclock().equals("LOCAL"))
			{
				this.client.sendDLLResyncCommand();
			}
			if(this.client.getBeammodeInt() == 6)
			{
				this.client.execGetfsdip();
			}
			if(this.client.getBeammodeInt() == 7 && 
					this.client.getMiclock().equals("BEAM1"))
			{
				this.client.execSctel("INF");
			}
			
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
			this.transButtonCtrl();
		}
		else if(evt.getPropertyName().equals("sendClock"))
		{
			this.transTo = (String)evt.getNewValue();
		}
		
		else if(evt.getPropertyName().equals("getShift"))
		{
			this.shift = (String) evt.getNewValue();
		}
		else if(evt.getPropertyName().equals("miclock"))
		{
			if(evt.getNewValue().equals("LOCAL"))
			{
				this.client.execSctel("MIN");
			}
		}
	}		
	
	/**
	 * Handels what do to when a ActionEvent is fired.
	 * Part of the ActionListener interface.
	 * @param evt
	 */
	public void actionPerformed(ActionEvent evt) 
	{
		if(evt.getSource() == this.autoButton)
		{
			if(!auto)
			{
				this.setEnableButtons(false);
				this.auto = true;
				this.client.sendOperChange("AUTO");
				this.autoMode(this.client.getBeammodeInt());
				
			}
		}
		else if (evt.getSource() == this.manuButton)
		{
			if(auto)
			{				
				this.setEnableButtons(true);
				this.auto = false;
				this.transButtonCtrl();
				this.client.sendOperChange("MANUAL");
			}
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
			this.resetShift();
		}
		else if(evt.getSource() == this.getShiftButton)
		{
			this.client.sendGetShift();
		}
	}
}

