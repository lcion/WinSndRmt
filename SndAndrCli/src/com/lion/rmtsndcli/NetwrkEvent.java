package com.lion.rmtsndcli;

public abstract class NetwrkEvent {
	public abstract void OnSuccessConnect();
	public abstract void OnFailedConnect();
	public abstract void OnSetVolume(byte b);
	public abstract void OnSetMute(boolean b);
}
