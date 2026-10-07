import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.*;

import java.util.Iterator;

public class SelectorServer {

    private static final int PORT = 8080;

    public static void main(String[] args) throws IOException {
        Selector selector = Selector.open();

        ServerSocketChannel serverSocketChannel = ServerSocketChannel.open();

        serverChannel.bind(new InetSocketAddress(PORT));

        serverChannel.configureBlocking(false);

        serverChannel.register(selector, SelectionKey.OP_ACCEPT);

        System.out.println("Selector server started on port " + PORT);

        while (true) {
            int ready = selector.select();

            System.out.println("ready events: " + ready);

            Iterator<SelectionKey> iterator = selector.selectedKeys().iterator();

            while (iterator.hasNext()) {
                SelectionKey key = itrator.next();

                iterator.remove();

                if (key.isAcceptable()) {

                    ServerSocketChannel server = (ServerSocketChannel) key.channel();

                    SocketChannel client = server.accept();

                    if (client == null){
                        continue;
                    }
                    client.configureBlocking(false);
                    System.out.println("client connected: " + client.getRemoteAddress());
                    client.register(selector, SelectionKey.OP_READ);
                }
            }

            if (key.isReadable()) {
                SocketChannel client = (SocketChannel) key.channel();

                ByteBuffer buffer = ByteBuffer.allocate(1024);

                int n;

                try {
                    n = client.read(buffer);
                } catch (IOException e) {
                    System.out.println("client reset/closed");

                    key.cancel();
                    client.close();

                    continue;
                }

                if (n == -1) {
                    System.out.println("client disconnected");

                    key.cancel();
                    client.close();
                    continue;
                }

                if (n > 0) {
                    buffer.flip();

                    byte[] data = new byte[buffer.remaining()];

                    buffer.get(data);

                    String message = new String(data);

                    System.out.println("recv: " + message);

                    buffer.clear();
                    buffer.put(data);
                    buffer.flip();

                    client.write(buffer);
                }
            }

        }
    }

}