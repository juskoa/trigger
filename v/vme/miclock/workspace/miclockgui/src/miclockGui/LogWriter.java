package miclockGui;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.sql.Timestamp;
import java.util.Date;

/**
 * Class for writing the log to a file.
 * @author oerjan
 *
 */
public class LogWriter {
	
	private File logFile;
	private BufferedWriter writer;
	
	/**
	 * Constructor
	 * 
	 */
	public LogWriter() {
		
		if(!Main.GUI_SHELL_MODE)
		{
			this.logFile = new File(Main.VMEWORK_DIR+"/WORK/miclock.log");
			if(this.logFile.exists())
			{
				Timestamp temp = new Timestamp(new Date().getTime());
				String ts = temp.toString().substring(2,4);
				ts += temp.toString().substring(5,7);
				ts += temp.toString().substring(8,10);
				ts += temp.toString().substring(11,13);
				ts += temp.toString().substring(14,16);
				this.logFile.renameTo(
						new File(Main.VMEWORK_DIR+"/WORK/miclock"+ts+".log"));
			}
			try {
				this.writer = new BufferedWriter(new FileWriter(this.logFile));
			} catch (IOException e) {
				e.printStackTrace();
			}
			
		}
	}
	
	/**
	 * Method that writes to file
	 * @param logEntry The String to write
	 */
	public void write(String logEntry)
	{
		try {
			this.writer.write(logEntry);
			this.writer.flush();
		} catch (IOException e) {
			e.printStackTrace();
		}
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

}
