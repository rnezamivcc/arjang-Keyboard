package com.wordlogic.acbuilder;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.util.ArrayList;

public class FileBuilder {
	public final static ByteOrder byteOrder = ByteOrder.LITTLE_ENDIAN;
	
	public int attributes;
	
	private ArrayList<FileBlob> blobs;
	
	public FileBuilder() {
		blobs = new ArrayList<FileBlob>();
		attributes = 0;
	}
	
	public void add(byte[] bb) {
		blobs.add(new FileBlob((bb)));
	}
	
	public void write(File file) throws IOException, FileNotFoundException {
		int c;
		FileOutputStream fos;
		byte[] header;
		ByteBuffer bbheader;
		byte[] size;
		ByteBuffer bbsize;
		int wrote;
		
		header = new byte[16];
		bbheader = ByteBuffer.wrap(header);
		
		bbheader.position(0);
		
		bbheader.order(byteOrder);
		
		bbheader.putInt(0x04030201); // magic number for endianness
		
		bbheader.putInt(5); // version code
		
		bbheader.putInt(blobs.size()); // block count
		
		bbheader.putInt(attributes); // attributes bitset
		
		fos = new FileOutputStream(file);

		fos.write(header);
		
		for(c = 0;c < blobs.size();c++) {
			size = new byte[4];
			
			bbsize = ByteBuffer.wrap(size);
			
			bbsize.position(0);
			
			bbsize.order(byteOrder);
			
			bbsize.putInt(blobs.get(c).data.length);
			
			fos.write(size);
			
			fos.write(blobs.get(c).data);
			
			wrote = blobs.get(c).data.length;
			
			while((wrote % 4) != 0) { // DWORD padding
				fos.write(0);
				wrote++;
			}
			
			System.out.println("wrote "+Integer.toString(blobs.get(c).data.length) + " bytes from blob");
		}
		
		fos.flush();
		
		fos.close();
	}
}