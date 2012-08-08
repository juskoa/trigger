package miclockGui;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

/**
 * Class for writing out Beammode, Miclock and Operation mode to a file. This
 * file is then read somewhere and put into a html file.
 * @author oerjan
 *
 */
public class HTMLWriter implements PropertyChangeListener{

	private MiClockClient client;
	private File htmlFile;
	private BufferedWriter writer;
	private String oper = null;

	// The lenght of a timeslot. change as needed
	private final int timeslotLength = 30;
	
	//used to check if clock is in transition
	private int intrans;
	
	//used to store the state which is being transitioned to
	private String transTo;
	
	/**
	 * Constructor
	 * @param client The MiClockClient for this program.
	 */
	public HTMLWriter(MiClockClient client) {
		this.client = client;
		this.client.addPropertyChangeListener(this);
		this.htmlFile = new File(Main.HTML_LOCATION);
	}
	
	/**
	 * Close the writer.
	 */
	public void close()
	{
		try {
			this.writer.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	/**
	 * Handels what to do when a Property Change Event is fired.
	 * Part of the PropertyChangeListener interface.
	 * Listenes for a change in the beammode and updates the html file with the
	 * new data.
	 * @param evt 
	 */
	public void propertyChange(PropertyChangeEvent evt) {
		// TODO Auto-generated method stub
		if(evt.getPropertyName().equals("beammode"))
		{
			if(this.oper != null)
			{
				String output;
				String beammode = this.client.getBeammodeString();
				String miclock = this.client.getMiclock();
				String ccm;
				if(this.oper.equals("MANUAL"))
				{
					ccm = "<FONT COLOR=\"red\">/man</FONT>";
				}
				else
				{
					ccm = "";
				}
				if(miclock.equals("LOCAL"))
				{
					/*
					 * changed 25.07s.12 from
					 output = "clock: <big><FONT COLOR=\"green\">" + beammode 
						+ "</FONT>" + ccm + "<br>";
					*/
					output = "clock: <big><FONT COLOR=\"green\">" + miclock 
						+ "</FONT>" + ccm + "<br>"; 
				}
				else if(miclock.equals("BEAM1"))
				{
					/*
					 * changed 25.07.12 from
					 * output = "clock: <big><FONT COLOR=\"blue\">" + beammode 
						+ "</FONT>" + ccm + "<br>";
					 */
					output = "clock: <big><FONT COLOR=\"blue\">" + miclock 
						+ "</FONT>" + ccm + "<br>";
				}
				else if(miclock.equals("BEAM2"))
				{
					/*
					 * changed 25.07.12 from
					 * output = "clock: <big><FONT COLOR=\"red\">" + beammode 
						+ "</FONT>" + ccm + "<br>";
					 */
					output = "clock: <big><FONT COLOR=\"red\">" + miclock 
						+ "</FONT>" + ccm + "<br>";
				}
				else
				{
					/*
					 * changed 25.07.12 from
					 * output = "clock: <big>" + beammode + "<br>";
					 */
					output = "clock: <big>" + miclock + "<br>";
				}
				
				if(this.intrans != 0)
				{
					
					String sec = "" + this.intrans * this.timeslotLength; 
					output += "(" + this.transTo + " in " + sec + "s)";
				}
				
				output += "</big>";
				try {
					this.writer = new BufferedWriter(new FileWriter(this.htmlFile));
					this.writer.write(output);
					this.writer.flush();
					this.writer.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
			}
		}
		else if(evt.getPropertyName().equals("sendOper"))
		{
			this.oper = (String) evt.getNewValue();
		}
		else if(evt.getPropertyName().equals("clocktrans"))
		{
			this.intrans = Integer.parseInt(this.client.getClocktrans());			
		}
		else if(evt.getPropertyName().equals("sendClock"))
		{
			this.transTo = (String)evt.getNewValue();
		}
	}

}
