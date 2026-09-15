package com.example.restservice;

import org.junit.jupiter.api.Test;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.resttestclient.autoconfigure.AutoConfigureRestTestClient;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.web.servlet.client.RestTestClient;

@SpringBootTest
@AutoConfigureRestTestClient
class CpuControllerTests {

  @Autowired
  private RestTestClient restTestClient;

  @Test
  void cpuWorkShouldReportTheTomcatWorker() {
    this.restTestClient.get().uri("/cpu?millis=1")
        .exchange()
        .expectStatus().isOk()
        .expectBody()
        .jsonPath("$.thread").value(thread ->
            ((String) thread).startsWith("http-nio-8080-exec-"))
        .jsonPath("$.elapsedMillis").isNumber()
        .jsonPath("$.iterations").isNumber();
  }

  @Test
  void cpuWorkShouldRejectAnUnsafeDuration() {
    this.restTestClient.get().uri("/cpu?millis=30001")
        .exchange()
        .expectStatus().isBadRequest();
  }
}
