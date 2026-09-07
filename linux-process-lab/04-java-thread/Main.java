public class Main{
    public static void main(String[] args) throws Exception {

        System.out.println(
            "PID = " + ProcessHandle.current().pid()
        );

        Thread t1 = new Thread(()-> {
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {
            }
        }, "worker-1");

        Thread t2 = new Thread(() -> {
            try {
                Thread.sleep(30000);
            } catch (InterruptedException e) {

            }
        }, "worker-2");

        t1.start();
        t2.start();
        
        t1.join();
        t2.join();
    }
}