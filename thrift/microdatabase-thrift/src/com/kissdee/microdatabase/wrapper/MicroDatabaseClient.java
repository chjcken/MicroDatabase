package com.kissdee.microdatabase.wrapper;

import com.kissdee.microdatabase.thrift.MicroDatabase;
import com.kissdee.microdatabase.thrift.TResult;
import java.nio.ByteBuffer;
import java.util.Deque;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.thrift.TException;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.transport.TFramedTransport;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TTransport;
import org.apache.thrift.transport.TTransportException;

/**
 *
 * @author datbt
 */
public class MicroDatabaseClient {

	private final String _host;
	private final int _port;
	private final ClientPool _clientPool;
	private final int _nretry;

	public MicroDatabaseClient(String host, int port) {
		this._host = host;
		this._port = port;
		this._clientPool = new ClientPool();
		this._nretry = 3;
	}

	public MicroDatabaseClient(String host, int port, int nretry) {
		this._host = host;
		this._port = port;
		this._clientPool = new ClientPool();
		this._nretry = nretry > 0 ? nretry : 3;
	}

	private final TResult NO_CONNECTION_RESULT = new TResult(ErrorCode.NO_CONNECTION.getValue());
	private final TResult BAD_REQUEST_RESULT = new TResult(ErrorCode.BAD_REQUEST.getValue());
	private final TResult BAD_CONNECTION_RESULT = new TResult(ErrorCode.BAD_CONNECTION.getValue());

	public TResult get(ByteBuffer key) {
		for (int i = 0; i < _nretry; i++) {
			MicroDatabase.Client cli = _clientPool.borrowClient();
			if (cli == null) {
				return NO_CONNECTION_RESULT;
			}
			try {
				TResult ret = cli.get(key);
				_clientPool.returnClient(cli);
				return ret;

			} catch (TTransportException ex) {
				Logger.getLogger(MicroDatabaseClient.class.getName()).log(Level.WARNING, ex.getMessage(), ex);
				continue;
			} catch (TException ex) {
				Logger.getLogger(MicroDatabaseClient.class.getName()).log(Level.SEVERE, ex.getMessage(), ex);
				return BAD_REQUEST_RESULT;
			} catch (Exception ex) {
				Logger.getLogger(MicroDatabaseClient.class.getName()).log(Level.SEVERE, ex.getMessage(), ex);
				return BAD_REQUEST_RESULT;
			}
		}
		return BAD_CONNECTION_RESULT;
	}

	public int exist(ByteBuffer key) {
		for (int i = 0; i < _nretry; i++) {
			MicroDatabase.Client cli = _clientPool.borrowClient();
			if (cli == null) {
				return ErrorCode.NO_CONNECTION.getValue();
			}
			try {
				int ret = cli.exist(key);
				_clientPool.returnClient(cli);
				return ret;

			} catch (TTransportException ex) {
				Logger.getLogger(MicroDatabaseClient.class.getName()).log(Level.WARNING, ex.getMessage(), ex);
				continue;
			} catch (TException ex) {
				Logger.getLogger(MicroDatabaseClient.class.getName()).log(Level.SEVERE, ex.getMessage(), ex);
				return ErrorCode.BAD_REQUEST.getValue();
			} catch (Exception ex) {
				Logger.getLogger(MicroDatabaseClient.class.getName()).log(Level.SEVERE, ex.getMessage(), ex);
				return ErrorCode.BAD_REQUEST.getValue();
			}
		}
		return ErrorCode.BAD_CONNECTION.getValue();
	}
	
	public int put(ByteBuffer key, ByteBuffer value) {
		for (int i = 0; i < _nretry; i++) {
			MicroDatabase.Client cli = _clientPool.borrowClient();
			if (cli == null) {
				return ErrorCode.NO_CONNECTION.getValue();
			}
			try {
				int ret = cli.put(key, value);
				_clientPool.returnClient(cli);
				return ret;

			} catch (TTransportException ex) {
				Logger.getLogger(MicroDatabaseClient.class.getName()).log(Level.WARNING, ex.getMessage(), ex);
				continue;
			} catch (TException ex) {
				Logger.getLogger(MicroDatabaseClient.class.getName()).log(Level.SEVERE, ex.getMessage(), ex);
				return ErrorCode.BAD_REQUEST.getValue();
			} catch (Exception ex) {
				Logger.getLogger(MicroDatabaseClient.class.getName()).log(Level.SEVERE, ex.getMessage(), ex);
				return ErrorCode.BAD_REQUEST.getValue();
			}
		}
		return ErrorCode.BAD_CONNECTION.getValue();
	}
	
	public int remove(ByteBuffer key) {
		for (int i = 0; i < _nretry; i++) {
			MicroDatabase.Client cli = _clientPool.borrowClient();
			if (cli == null) {
				return ErrorCode.NO_CONNECTION.getValue();
			}
			try {
				int ret = cli.remove(key);
				_clientPool.returnClient(cli);
				return ret;

			} catch (TTransportException ex) {
				Logger.getLogger(MicroDatabaseClient.class.getName()).log(Level.WARNING, ex.getMessage(), ex);
				continue;
			} catch (TException ex) {
				Logger.getLogger(MicroDatabaseClient.class.getName()).log(Level.SEVERE, ex.getMessage(), ex);
				return ErrorCode.BAD_REQUEST.getValue();
			} catch (Exception ex) {
				Logger.getLogger(MicroDatabaseClient.class.getName()).log(Level.SEVERE, ex.getMessage(), ex);
				return ErrorCode.BAD_REQUEST.getValue();
			}
		}
		return ErrorCode.BAD_CONNECTION.getValue();
	}

	private class ClientPool {

		private final Deque<MicroDatabase.Client> _pool;

		public ClientPool() {
			this._pool = new LinkedBlockingDeque<>();
		}

		private MicroDatabase.Client createNewClient() {
			try {
				TSocket socket = new TSocket(_host, _port);
				TTransport transport = new TFramedTransport(socket);
				transport.open();
				TProtocol protocol = new TBinaryProtocol(transport);

				return new MicroDatabase.Client(protocol);
			} catch (TTransportException ex) {
				return null;
			}
		}

		public MicroDatabase.Client borrowClient() {
			MicroDatabase.Client ret = _pool.poll();
			if (ret == null) {
				ret = createNewClient();
			}
			return ret;
		}

		public boolean returnClient(MicroDatabase.Client cli) {
			if (cli != null) {
				return _pool.offer(cli);
			}
			return false;
		}
	}
}
