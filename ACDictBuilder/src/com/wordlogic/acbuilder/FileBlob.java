package com.wordlogic.acbuilder;

import java.nio.ByteBuffer;

public class FileBlob {
	public final byte[] data;
	public int padding;
	
	public FileBlob(byte[] bytes) {
		data = bytes;
		padding = 0;
	}
}