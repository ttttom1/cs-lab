# Day 5: Spring Boot and Tomcat worker threads

## 1. Select Java 17

The machine's default Java is 11, while this example requires Java 17. Change
Java only in the current shell:

```bash
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
export PATH="$JAVA_HOME/bin:$PATH"
java -version
```

## 2. Build and run

```bash
cd /root/workspace/linux-process-lab/05-spring-tomcat
./mvnw test package
java -jar target/rest-service-complete-0.0.1-SNAPSHOT.jar
```

Keep this terminal open. In a second terminal, verify the API:

```bash
curl -s 'http://localhost:8080/greeting?name=Linux'
curl -s 'http://localhost:8080/cpu?millis=1000'
```

The `/cpu` response includes the Tomcat worker name that ran the Controller.

## 3. Find the JVM and inspect its workers

```bash
jps -l
jcmd <PID> Thread.print | less
```

Search for `http-nio-8080-exec-`. Those are embedded Tomcat HTTP workers. An
active CPU request also shows `CpuController.cpu` in that worker's stack.

## 4. Create runnable workers and observe scheduling

Start 100 CPU-bound HTTP requests in a third terminal:

```bash
seq 1 100 | xargs -P 100 -I{} \
  curl -s 'http://localhost:8080/cpu?millis=10000' -o /dev/null
```

While they run, inspect the JVM from the second terminal:

```bash
top -H -p <PID>
jcmd <PID> Thread.print > /tmp/tomcat-threads.txt
grep -A 15 'http-nio-8080-exec-' /tmp/tomcat-threads.txt | less
```

This machine has four CPU cores. With many runnable workers, the scheduler
rotates more runnable threads across those cores. A context switch saves and
restores execution state such as registers, the program counter, and the stack
pointer.

## 5. Stop safely

```bash
kill <PID>
```

Use the exact PID reported by `jps -l`; do not use a broad `pkill java` command.
