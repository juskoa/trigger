package miclockGui;

import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.sql.Timestamp;
import java.util.Date;

import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.ScrollPaneConstants;
import javax.swing.text.DefaultCaret;

/**
 * A panel containing all of the information fields used in this program.
 * @author oerjan
 *
 */
public class InfoPanel extends JPanel implements PropertyChangeListener 
{
	
	private final static int logLength = 100;
	
	private JLabel beammodeLabel;
	private JLabel currentClockLabel;
	private JLabel logLabel;
	private JLabel shiftLabel;
	
	private JTextField beammodeField;
	private JTextField miclockField;
	private JTextField shiftField;
	
	private JTextArea logArea;
	private JScrollPane scrollPane;
	
	private MiClockClient client;
	private LogWriter logWriter;
	
	private GridBagConstraints constraints;
	
	/**
	 * Constructor
	 * @param client The MiClockClient for this program.
	 * @param logWriter LogWriter which writes the Log File.
	 */
	public InfoPanel(MiClockClient client, LogWriter logWriter)
	{
		this.client = client;
		this.logWriter = logWriter;
		this.client.addPropertyChangeListener(this);
		
		this.setLayout(new GridBagLayout());
		this.constraints = new GridBagConstraints();
		
		this.constraints.gridx = 0;
		this.constraints.gridy = 0;
		this.constraints.gridheight = 1;
		this.constraints.gridwidth = 1;
		this.constraints.fill = GridBagConstraints.HORIZONTAL;
		this.constraints.weightx = 0.5;
		this.constraints.weighty = 0.5;
		
		this.beammodeLabel = new JLabel("Beammode:");
		this.currentClockLabel = new JLabel("Current clock:");
		this.shiftLabel = new JLabel("Shift:");
		this.logLabel = new JLabel("Log:");
		
		this.beammodeField = new JTextField();
		this.miclockField = new JTextField();
		this.shiftField = new JTextField();
		
		this.beammodeField.setEditable(false);
		this.miclockField.setEditable(false);
		this.shiftField.setEditable(false);
		
		this.logArea = new JTextArea();
		this.logArea.setEditable(false);
		this.logArea.setText("");
		this.logArea.setWrapStyleWord(true);
		this.logArea.setLineWrap(true);
		this.logArea.setFont(new Font("monospaced", Font.PLAIN, 13));
		this.scrollPane = new JScrollPane(this.logArea);
		this.scrollPane.setVerticalScrollBarPolicy(
				ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
		
		this.add(this.beammodeLabel,this.constraints);
		this.constraints.gridy = 1;
		this.add(this.currentClockLabel,this.constraints);
		this.constraints.gridy = 2;
		this.add(this.shiftLabel, this.constraints);
		this.constraints.gridy = 3;
		this.add(this.logLabel, this.constraints);
		
		this.constraints.gridx = 1;
		this.constraints.gridy = 0;
		this.constraints.weightx = 8.5;
		this.add(this.beammodeField, this.constraints);
		this.constraints.gridy = 1;
		this.add(this.miclockField , this.constraints);
		this.constraints.gridy = 2;
		this.add(this.shiftField, this.constraints);
		this.constraints.gridy = 3;
		this.constraints.weighty = 7.5;
		this.constraints.fill = GridBagConstraints.BOTH;
		this.add(this.scrollPane, this.constraints);
		
		//To get the scrollPane to autoscroll
		DefaultCaret caret = (DefaultCaret)this.logArea.getCaret();
		caret.setUpdatePolicy(DefaultCaret.ALWAYS_UPDATE);
	}
	
	/** 
	 * check the length of the log and removes lines longer then
	 * InfoPanel.logLength.
	 */
	private void checkLogLength()
	{
		if(this.logArea.getText().length() > 0)
		{
			String[] log = this.logArea.getText().split("\r\n|\r|\n");
			
			if(log.length >= InfoPanel.logLength)
			{
				String temp = "";
				for(int i = 1; i < log.length; i++)
				{
					temp += log[i] + "\n";
				}
				this.logArea.setText(temp);
			}
		}
	}
	
	/** 
	 * Creates a string with a timestamp formated as HH:MM:SS 
	 * for the logArea.
	 */
	private String createAreaTS()
	{
		return new Timestamp(new Date().getTime()).toString().substring(11,19);
	}
	
	/**
	 * Creates a string with a timestam formated as YYYY-MM-DD HH:MM:SS
	 * for the log file.
	 */
	private String createFileTS()
	{
		return new Timestamp(new Date().getTime()).toString().substring(0,19);
	}
	
	/**
	 * Handles what to do when a Property Change Event is fired.
	 * The main functionality of this method is to set the information in the
	 * JTextFields and the Log Area, and to send information of what to write
	 * in the Log File.
	 * Part of the PropertyChangeListener interface.
	 * @param evt 
	 */
	public void propertyChange(PropertyChangeEvent evt) {
		
		this.checkLogLength();
		String logAreaTS = this.createAreaTS();
		String logFileTS = this.createFileTS();
		String logEntry = "";
		boolean write = false;
		
		if(evt.getPropertyName().equals("clocktrans"))
		{
			logEntry = ": Clock transition in " 
				+ this.client.getClocktrans() + " timeslots\n";
			write = true;
		}
		else if(evt.getPropertyName().equals("beammode"))
		{
			this.beammodeField.setText(this.client.getBeammodeString());
			logEntry = ": Beammode changed to " 
				+ this.client.getBeammodeString() + "\n";
			write = true;
		}
		else if(evt.getPropertyName().equals("miclock"))
		{
			this.miclockField.setText(this.client.getMiclock());
			logEntry = ": Current clock changed to " 
				+ this.client.getMiclock() + "\n";
			write = true;
		}
		else if(evt.getPropertyName().equals("sendClock"))
		{
			logEntry = ": Sent command "+evt.getNewValue()+" to MICLOCK_SET\n";
			write = true;
		}
		else if(evt.getPropertyName().equals("sendShift"))
		{
			logEntry = ": Sent command "+evt.getNewValue()+" to CORDE_SET\n";
			write = true;
		}
		else if(evt.getPropertyName().equals("sendMode"))
		{
			logEntry = ": Mode changed to " + evt.getNewValue() + "\n";
			write = true;
		}
		else if(evt.getPropertyName().equals("getShift"))
		{
			String shift = (String) evt.getNewValue();
			this.shiftField.setText(shift + " ns");
			logEntry = ": Current shift is " + shift+" ns\n";
			write = true;
		}
		
		if(write)
		{			
			this.logWriter.write(logFileTS + logEntry);
			this.logArea.append(logAreaTS+logEntry);
		}
		
	}
}
