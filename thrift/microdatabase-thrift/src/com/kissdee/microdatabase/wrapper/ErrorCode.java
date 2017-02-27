/*
 * Copyright (c) 2012-2016 by Zalo Group.
 * All Rights Reserved.
 */
package com.kissdee.microdatabase.wrapper;

/**
 *
 * @author datbt
 */
public enum ErrorCode {
	//logic error
	SUCCESS(0),
	FAIL (1),
	NOT_EXIST(2),
	PARAM_OVERSIZE(3),
	
	//network error
	NO_CONNECTION(1000),
	BAD_CONNECTION(1001),
	BAD_REQUEST(1002);
	
	
	private final int value;
	
	ErrorCode(int val) {
		this.value = val;
	}
	
	public int getValue() {
		return -this.value;
	}
}
