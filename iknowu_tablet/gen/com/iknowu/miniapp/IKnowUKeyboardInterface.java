/*
 * This file is auto-generated.  DO NOT MODIFY.
 * Original file: C:\\Users\\Reza\\Dropbox\\Photos\\code\\arjangkb\\iknowu_tablet\\src\\com\\iknowu\\miniapp\\IKnowUKeyboardInterface.aidl
 */
package com.iknowu.miniapp;
public interface IKnowUKeyboardInterface extends android.os.IInterface
{
/** Local-side IPC implementation stub class. */
public static abstract class Stub extends android.os.Binder implements com.iknowu.miniapp.IKnowUKeyboardInterface
{
private static final java.lang.String DESCRIPTOR = "com.iknowu.miniapp.IKnowUKeyboardInterface";
/** Construct the stub at attach it to the interface. */
public Stub()
{
this.attachInterface(this, DESCRIPTOR);
}
/**
 * Cast an IBinder object into an com.iknowu.miniapp.IKnowUKeyboardInterface interface,
 * generating a proxy if needed.
 */
public static com.iknowu.miniapp.IKnowUKeyboardInterface asInterface(android.os.IBinder obj)
{
if ((obj==null)) {
return null;
}
android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
if (((iin!=null)&&(iin instanceof com.iknowu.miniapp.IKnowUKeyboardInterface))) {
return ((com.iknowu.miniapp.IKnowUKeyboardInterface)iin);
}
return new com.iknowu.miniapp.IKnowUKeyboardInterface.Stub.Proxy(obj);
}
@Override public android.os.IBinder asBinder()
{
return this;
}
@Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
{
switch (code)
{
case INTERFACE_TRANSACTION:
{
reply.writeString(DESCRIPTOR);
return true;
}
case TRANSACTION_sendText:
{
data.enforceInterface(DESCRIPTOR);
java.lang.String _arg0;
_arg0 = data.readString();
int _arg1;
_arg1 = data.readInt();
int _arg2;
_arg2 = data.readInt();
boolean _arg3;
_arg3 = (0!=data.readInt());
this.sendText(_arg0, _arg1, _arg2, _arg3);
reply.writeNoException();
return true;
}
case TRANSACTION_updateView:
{
data.enforceInterface(DESCRIPTOR);
android.widget.RemoteViews _arg0;
if ((0!=data.readInt())) {
_arg0 = android.widget.RemoteViews.CREATOR.createFromParcel(data);
}
else {
_arg0 = null;
}
int _arg1;
_arg1 = data.readInt();
this.updateView(_arg0, _arg1);
reply.writeNoException();
return true;
}
case TRANSACTION_clip:
{
data.enforceInterface(DESCRIPTOR);
java.lang.String _arg0;
_arg0 = data.readString();
this.clip(_arg0);
reply.writeNoException();
return true;
}
case TRANSACTION_close:
{
data.enforceInterface(DESCRIPTOR);
this.close();
reply.writeNoException();
return true;
}
case TRANSACTION_deleteChars:
{
data.enforceInterface(DESCRIPTOR);
int _arg0;
_arg0 = data.readInt();
int _arg1;
_arg1 = data.readInt();
this.deleteChars(_arg0, _arg1);
reply.writeNoException();
return true;
}
}
return super.onTransact(code, data, reply, flags);
}
private static class Proxy implements com.iknowu.miniapp.IKnowUKeyboardInterface
{
private android.os.IBinder mRemote;
Proxy(android.os.IBinder remote)
{
mRemote = remote;
}
@Override public android.os.IBinder asBinder()
{
return mRemote;
}
public java.lang.String getInterfaceDescriptor()
{
return DESCRIPTOR;
}
/**
	 * Send a String of text to the keyboard,
	 * that will be posted to the input box upon being received
	 * @param param: The text you would like to put into the EditText field
	 */
@Override public void sendText(java.lang.String param, int before, int after, boolean stayAlive) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
_data.writeString(param);
_data.writeInt(before);
_data.writeInt(after);
_data.writeInt(((stayAlive)?(1):(0)));
mRemote.transact(Stub.TRANSACTION_sendText, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
/**
	 * Tell the keyboard that your RemoteViews object needs updating
	 * @param rm: Your updated RemoteViews object that will be displayed
	 */
@Override public void updateView(android.widget.RemoteViews rm, int anim) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
if ((rm!=null)) {
_data.writeInt(1);
rm.writeToParcel(_data, 0);
}
else {
_data.writeInt(0);
}
_data.writeInt(anim);
mRemote.transact(Stub.TRANSACTION_updateView, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
/**
	 * Tell the keyboard that you want to post some text to the ClipBoard
	 * @param param: The text you would like to clip
	 */
@Override public void clip(java.lang.String param) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
_data.writeString(param);
mRemote.transact(Stub.TRANSACTION_clip, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
/**
	 * Tell the keyboard that you are done and would like it to close your connection
	 * as well as the mini-app drawer.
	 */
@Override public void close() throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
mRemote.transact(Stub.TRANSACTION_close, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
/**
	 * Delete a specified number of characters from the text.
	 * @param before: the number of characters to delete before the current cursor position
	 * @param afterCursor: the number of characters to delete after the current cursor position
	 */
@Override public void deleteChars(int before, int after) throws android.os.RemoteException
{
android.os.Parcel _data = android.os.Parcel.obtain();
android.os.Parcel _reply = android.os.Parcel.obtain();
try {
_data.writeInterfaceToken(DESCRIPTOR);
_data.writeInt(before);
_data.writeInt(after);
mRemote.transact(Stub.TRANSACTION_deleteChars, _data, _reply, 0);
_reply.readException();
}
finally {
_reply.recycle();
_data.recycle();
}
}
}
static final int TRANSACTION_sendText = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
static final int TRANSACTION_updateView = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
static final int TRANSACTION_clip = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
static final int TRANSACTION_close = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
static final int TRANSACTION_deleteChars = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
}
/**
	 * Send a String of text to the keyboard,
	 * that will be posted to the input box upon being received
	 * @param param: The text you would like to put into the EditText field
	 */
public void sendText(java.lang.String param, int before, int after, boolean stayAlive) throws android.os.RemoteException;
/**
	 * Tell the keyboard that your RemoteViews object needs updating
	 * @param rm: Your updated RemoteViews object that will be displayed
	 */
public void updateView(android.widget.RemoteViews rm, int anim) throws android.os.RemoteException;
/**
	 * Tell the keyboard that you want to post some text to the ClipBoard
	 * @param param: The text you would like to clip
	 */
public void clip(java.lang.String param) throws android.os.RemoteException;
/**
	 * Tell the keyboard that you are done and would like it to close your connection
	 * as well as the mini-app drawer.
	 */
public void close() throws android.os.RemoteException;
/**
	 * Delete a specified number of characters from the text.
	 * @param before: the number of characters to delete before the current cursor position
	 * @param afterCursor: the number of characters to delete after the current cursor position
	 */
public void deleteChars(int before, int after) throws android.os.RemoteException;
}
