package com.example.restservice;

import java.util.concurrent.TimeUnit;

import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.server.ResponseStatusException;

@RestController
public class CpuController {

  private static final long MAX_MILLIS = 30_000;

  @GetMapping("/cpu")
  public CpuResult cpu(@RequestParam(defaultValue = "5000") long millis) {
    if (millis < 1 || millis > MAX_MILLIS) {
      throw new ResponseStatusException(
          HttpStatus.BAD_REQUEST, "millis must be between 1 and 30000");
    }

    long startedAt = System.nanoTime();
    long deadline = startedAt + TimeUnit.MILLISECONDS.toNanos(millis);
    long iterations = 0;
    long checksum = 0;

    while (System.nanoTime() < deadline) {
      checksum = Long.rotateLeft(checksum ^ (iterations * 0x9E3779B97F4A7C15L), 13);
      iterations++;
    }

    long elapsedMillis = TimeUnit.NANOSECONDS.toMillis(System.nanoTime() - startedAt);
    return new CpuResult(Thread.currentThread().getName(), elapsedMillis, iterations, checksum);
  }

  public record CpuResult(
      String thread, long elapsedMillis, long iterations, long checksum) { }
}
