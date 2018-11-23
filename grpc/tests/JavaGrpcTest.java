/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import MyGame.Example.Monster;
import MyGame.Example.MonsterStorageGrpc;
import MyGame.Example.Stat;
import com.google.flatbuffers.FlatBufferBuilder;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.Server;
import io.grpc.ServerBuilder;
import io.grpc.stub.StreamObserver;
import org.junit.Assert;

import java.io.IOException;
import java.lang.InterruptedException;
import java.nio.ByteBuffer;
import java.util.Iterator;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.CountDownLatch;


/**
 * Demonstrates basic client-server interaction using grpc-java over netty.
 */
public class JavaGrpcTest {
    static final String BIG_MONSTER_NAME = "Cyberdemon";
    static final short nestedMonsterHp = 600;
    static final short nestedMonsterMana = 1024;
    static final int numStreamedMsgs = 10;
    static final int timeoutMs = 3000;
    static Server server;
    static ManagedChannel channel;
    static MonsterStorageGrpc.MonsterStorageBlockingStub blockingStub;
    static MonsterStorageGrpc.MonsterStorageStub asyncStub;

    static class MyService extends MonsterStorageGrpc.MonsterStorageImplBase {
        @Override
        public void store(Monster request, io.grpc.stub.StreamObserver<Stat> responseObserver) {
            Assert.assertEquals(request.name(), BIG_MONSTER_NAME);
            Assert.assertEquals(request.hp(), nestedMonsterHp);
            Assert.assertEquals(request.mana(), nestedMonsterMana);
            System.out.println("Received store request from " + request.name());
            // Create a response from the incoming request name.
            Stat stat = GameFactory.createStat("Hello " + request.name(), 100, 10);
            responseObserver.onNext(stat);
            responseObserver.onCompleted();
        }

        @Override
        public void retrieve(Stat request, io.grpc.stub.StreamObserver<Monster> responseObserver) {
            // Create 10 monsters for streaming response.
            for (int i=0; i<numStreamedMsgs; i++) {
                Monster monster = GameFactory.createMonsterFromStat(request, i);
                responseObserver.onNext(monster);
            }
            responseObserver.onCompleted();
        }

        @Override
        public StreamObserver<Monster> getMaxHitPoint(final StreamObserver<Stat> responseObserver) {
          final AtomicInteger maxHp = new AtomicInteger();
          final AtomicReference<String> maxHpMonsterName = new AtomicReference<String>();
          final AtomicInteger maxHpCount = new AtomicInteger();

          return new StreamObserver<Monster>() {
            public void onNext(Monster monster) {
              if (monster.hp() > maxHp.get()) {
                // Found a monster of higher hit points. Store its hp, name. Reset count to 1.
                maxHp.set(monster.hp());
                maxHpMonsterName.set(monster.name()); 
                maxHpCount.set(1);
              }
              else if (monster.hp() == maxHp.get()) {
                // Count how many times we saw a monster of current max hit points.
                maxHpCount.getAndIncrement();
              }
            }
            public void onCompleted() {
              Stat maxHpStat = GameFactory.createStat(maxHpMonsterName.get(), maxHp.get(), maxHpCount.get());
              responseObserver.onNext(maxHpStat);
              responseObserver.onCompleted();
            }
            public void onError(Throwable t) {
              // Not expected
              Assert.fail();
            };
          };
        }
    }

    @org.junit.BeforeClass
    public static void startServer() throws IOException {
        server = ServerBuilder.forPort(0).addService(new MyService()).build().start();
        int port = server.getPort();
        channel = ManagedChannelBuilder.forAddress("localhost", port)
                // Channels are secure by default (via SSL/TLS). For the example we disable TLS to avoid
                // needing certificates.
                .usePlaintext(true)
                .directExecutor()
                .build();
        blockingStub = MonsterStorageGrpc.newBlockingStub(channel);
        asyncStub = MonsterStorageGrpc.newStub(channel);
    }

    @org.junit.Test
    public void testUnary() throws IOException {
        Monster monsterRequest = GameFactory.createMonster(BIG_MONSTER_NAME, nestedMonsterHp, nestedMonsterMana);
        Stat stat = blockingStub.store(monsterRequest);
        Assert.assertEquals(stat.id(), "Hello " + BIG_MONSTER_NAME);
        System.out.println("Received stat response from service: " + stat.id());
    }

    @org.junit.Test
    public void testServerStreaming() throws IOException {
        Monster monsterRequest = GameFactory.createMonster(BIG_MONSTER_NAME, nestedMonsterHp, nestedMonsterMana);
        Stat stat = blockingStub.store(monsterRequest);
        Iterator<Monster> iterator = blockingStub.retrieve(stat);
        int counter = 0;
        while(iterator.hasNext()) {
            Monster m = iterator.next();
            System.out.println("Received monster " + m.name());
            counter ++;
        }
        Assert.assertEquals(counter, numStreamedMsgs);
        System.out.println("FlatBuffers GRPC client/server test: completed successfully");
    }

    @org.junit.Test
    public void testClientStream() throws IOException, InterruptedException {
      final AtomicReference<Stat> maxHitStat = new AtomicReference<Stat>();
      final CountDownLatch streamAlive = new CountDownLatch(1);

      StreamObserver<Stat> statObserver = new StreamObserver<Stat>() {
        public void onCompleted() { 
          streamAlive.countDown();
        }
        public void onError(Throwable ex) { }
        public void onNext(Stat stat) {
          maxHitStat.set(stat);
        }
      };
      StreamObserver<Monster> monsterStream = asyncStub.getMaxHitPoint(statObserver);
      short count = 10;
      for (short i = 0;i < count; ++i) {
        Monster monster = GameFactory.createMonster(BIG_MONSTER_NAME + i, (short) (nestedMonsterHp * i), nestedMonsterMana);
        monsterStream.onNext(monster);
      }
      monsterStream.onCompleted();
      // Wait a little bit for the server to send the stats of the monster with the max hit-points.
      streamAlive.await(timeoutMs, TimeUnit.MILLISECONDS);
      Assert.assertEquals(maxHitStat.get().id(), BIG_MONSTER_NAME + (count - 1));
      Assert.assertEquals(maxHitStat.get().val(), nestedMonsterHp * (count - 1));
      Assert.assertEquals(maxHitStat.get().count(), 1);
    }
}
