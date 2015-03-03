
	void recursiveVPrint(char* buf,unsigned int offset,string s) {
		char ch = buf[offset];
		int nchildren = buf[offset+1];
		int c;
		unsigned int* op = (unsigned int*)(buf+offset+2);
		string ns;

		if(ch != 0) { ns = s + ch; }
		else { ns = s; }

		LOGI(ns.c_str());

		for(c = 0;c < nchildren;c++) {
			recursiveVPrint(buf,op[c],ns);
		}
	}

	void recursiveVPrintB(char* buf,unsigned int offset,string s,unsigned int* hashes,int nhashes,unsigned char* probs) {
		char ch = buf[offset];
		int nchildren = getNChildren(buf,offset);
		int c;
		string ns;
		unsigned int co;
		unsigned int hash;
		int p;

		if(ch != 0) { ns = s + ch; }
		else { ns = s; }

		hash = wordHash(ns.c_str());

		p = getProb(hash,hashes,nhashes,probs);

		if(p > 0 && ((p & 128) == 128)) {
		LOGI("%s p = %d",ns.c_str(),p);
		}

		for(c = 0;c < nchildren;c++) {
			co = getChild(buf,offset,c);
			recursiveVPrintB(buf,co,ns,hashes,nhashes,probs);
		}
	}

	void VPrint(char* buf,unsigned int offset) {
		int nchildren;
		int c;
		unsigned int* ip;

		nchildren = buf[offset+1];

		LOGI("vertex at offset %d",offset);
		LOGI("character = %c",buf[offset]);
		LOGI("has %d children",nchildren);

		ip = (unsigned int*)(buf + offset + 2);

		for(c = 0;c < nchildren;c++) {
			LOGI("offset %d",ip[c]);
		}
	}

	JNIEXPORT void Java_com_iknowu_autocorrect_AutoCorrect_walkGraph(JNIEnv* env,jclass cls,jobject dbuf) {
		unsigned int* bufi;
		string s;
		char* graphbuf;
		unsigned int* hashes;
		unsigned int nhashes;
		unsigned char* probs;
		char* p;

		bufi = (unsigned int*)env->GetDirectBufferAddress(dbuf);
		if(env->ExceptionCheck() || bufi == NULL) { return; }

		graphbuf = ((char*)bufi) + bufi[3];

		nhashes = bufi[6];
		p = (char*)bufi + bufi[2];
		hashes = (unsigned int*)p;

		p = (char*)bufi + bufi[4];
		probs = (unsigned char*)p;

		recursiveVPrintB(graphbuf,0,s,hashes,nhashes,probs);
	}

	JNIEXPORT void Java_com_iknowu_autocorrect_AutoCorrect_twoGramInfo(JNIEnv* env,jclass cls,jstring str,jobject dbuf) {
		const char* s;
		unsigned int* bufi;
		char* p;

		unsigned int* twograms;
		unsigned int ntwograms;
		unsigned short* twogramis;
		unsigned int* hashes;
		unsigned int nhashes;
		unsigned int hash;

		int hindex;

		int tgindex;
		int tgcount;
		int c;

		s = env->GetStringUTFChars(str,NULL);
		if(env->ExceptionCheck()) { return; }

		bufi = (unsigned int*)env->GetDirectBufferAddress(dbuf);
		if(env->ExceptionCheck() || bufi == NULL) { return; }

		// read offset data from the dictionary file so we can get the graph, hash table, and probability pointers
		nhashes = bufi[6];
		p = (char*)bufi + bufi[2];
		hashes = (unsigned int*)p;

		/*p = (char*)bufi + bufi[3];
		graph = p;

		p = (char*)bufi + bufi[4];
		prob = (unsigned char*)p;*/

		ntwograms = bufi[9];
		p = (char*)bufi + bufi[7];
		twograms = (unsigned int*)p;

		p = (char*)bufi + bufi[8];
		twogramis = (unsigned short*)p;

		hash = wordHash(s);

		hindex = getIndex(hash,hashes,nhashes);

		LOGI("hash index = %d",hindex);

		tgindex = twogramis[hindex*2];
		tgcount = twogramis[1+(getIndex(hash,hashes,nhashes)*2)];

		LOGI("for the word %s there are %d twograms starting with index %d",s,tgcount,tgindex);

		for(c = 0;c < tgcount;c++) {
			LOGI("%x",twograms[tgindex+c]);
		}

		env->ReleaseStringUTFChars(str,s);
		if(env->ExceptionCheck()) { return; }
	}

	JNIEXPORT void Java_com_iknowu_autocorrect_AutoCorrect_test(JNIEnv* env,jobject obj) {
		jfieldID fidBufKL;
		jobject bufKL;
		unsigned char* bufkeyboard;
		kbdDistance* kbd;
		jclass clsAC;

		clsAC = env->GetObjectClass(obj);
		if(env->ExceptionCheck()) { return; }

		fidBufKL = env->GetFieldID(clsAC,"bufKL","Ljava/nio/ByteBuffer;");
		if(env->ExceptionCheck()) {
#if debug
			LOGE("Exception at GetFieldID(clsAC,\"bufKL\",\"Ljava/nio/ByteBuffer\") in test %s line %d",__FILE__,__LINE__);
#endif
			return;
		}

		bufKL = env->GetObjectField(obj,fidBufKL);
		if(env->ExceptionCheck()) { return; }

		if(bufKL != NULL) {
			bufkeyboard = (unsigned char*)env->GetDirectBufferAddress(bufKL);
			if(env->ExceptionCheck()) { kbd = NULL; } // we don't need to stop for this
			kbd = new kbdDistance(bufkeyboard);
		} else {
			kbd = NULL;
		}

		LOGI("kbd->dist(k,i) = %f",kbd->dist('k','i'));
		LOGI("kbd->dist(s,t) = %f",kbd->dist('s','t'));
		LOGI("kbd->dist(i,m) = %f",kbd->dist('i','m'));
	}
