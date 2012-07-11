package miclockGui;

import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

/**
 * 
 * @author oerjan
 *
 */
public class HTMLWriter implements PropertyChangeListener{

	private MiClockClient client;
	private File htmlFile;
	private BufferedWriter writer;
	private String mode = null;
	
	public HTMLWriter(MiClockClient client) {
		// TODO Auto-generated constructor stub
		this.client = client;
		this.client.addPropertyChangeListener(this);
		
		this.htmlFile = new File(Main.HTML_LOCATION);
		try {
			this.writer = new BufferedWriter(new FileWriter(this.htmlFile));
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public void propertyChange(PropertyChangeEvent evt) {
		// TODO Auto-generated method stub
		if(evt.getPropertyName().equals("beammode"))
		{
			if(mode != null)
			{
				String output;
				if(mode.equals(""))
				{
					
				}
			}
		}
		else if(evt.getPropertyName().equals("sendMode"))
		{
			this.mode = (String) evt.getNewValue();
		}
	}

}
