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
import org.junit.Assert;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Iterator;


/**
 * Demonstrates basic client-server interaction using grpc-java over netty.
 */
public class JavaGrpcTest {
    static final String BIG_MONSTER_NAME = "big-monster";
    static final short nestedMonsterHp = 600;
    static final short nestedMonsterMana = 1024;
    static final int numStreamedMsgs = 10;

    static class MyService extends MonsterStorageGrpc.MonsterStorageImplBase {
        @Override
        public void store(Monster request, io.grpc.stub.StreamObserver<Stat> responseObserver) {
            Assert.assertEquals(request.name(), BIG_MONSTER_NAME);
            Assert.assertEquals(request.hp(), nestedMonsterHp);
            Assert.assertEquals(request.mana(), nestedMonsterMana);
            System.out.println("Received store request from " + request.name());
            // Create a response from the incoming request name.
            FlatBufferBuilder builder = new FlatBufferBuilder();
            int statOffset = Stat.createStat(builder, builder.createString("Hello " + request.name()), 100, 10);
            builder.finish(statOffset);
            Stat stat = Stat.getRootAsStat(builder.dataBuffer());
            responseObserver.onNext(stat);
            responseObserver.onCompleted();
        }

        @Override
        public void retrieve(Stat request, io.grpc.stub.StreamObserver<Monster> responseObserver) {
            // Create 10 monsters for streaming response.
            for (int i=0; i<numStreamedMsgs; i++) {
                FlatBufferBuilder builder = new FlatBufferBuilder();
                int i1 = builder.createString(request.id() + " No." + i);
                Monster.startMonster(builder);
                Monster.addName(builder, i1);
                int i2 = Monster.endMonster(builder);
                Monster.finishMonsterBuffer(builder, i2);
                Monster monster = Monster.getRootAsMonster(builder.dataBuffer());
                responseObserver.onNext(monster);
            }
            responseObserver.onCompleted();
        }
    }


    private static int startServer() throws IOException {
        Server server = ServerBuilder.forPort(0).addService(new MyService()).build().start();
        return server.getPort();
    }

    @org.junit.Test
    public void testMonster() throws IOException {
        int port = startServer();
        ManagedChannel channel = ManagedChannelBuilder.forAddress("localhost", port)
                // Channels are secure by default (via SSL/TLS). For the example we disable TLS to avoid
                // needing certificates.
                .usePlaintext(true)
                .directExecutor()
                .build();

        MonsterStorageGrpc.MonsterStorageBlockingStub stub = MonsterStorageGrpc.newBlockingStub(channel);

        FlatBufferBuilder builder = new FlatBufferBuilder();

        int o_string = builder.createString(BIG_MONSTER_NAME);
        Monster.startMonster(builder);
        Monster.addName(builder, o_string);
        Monster.addHp(builder, nestedMonsterHp);
        Monster.addMana(builder, nestedMonsterMana);
        int monster1 = Monster.endMonster(builder);
        Monster.finishMonsterBuffer(builder, monster1);

        ByteBuffer buffer = builder.dataBuffer();
        Monster monsterRequest = Monster.getRootAsMonster(buffer);
        Stat stat = stub.store(monsterRequest);
        Assert.assertEquals(stat.id(), "Hello " + BIG_MONSTER_NAME);
        System.out.println("Received stat response from service: " + stat.id());


        Iterator<Monster> iterator = stub.retrieve(stat);
        int counter = 0;
        while(iterator.hasNext()) {
            Monster m = iterator.next();
            System.out.println("Received monster " + m.name());
            counter ++;
        }
        Assert.assertEquals(counter, numStreamedMsgs);
        System.out.println("FlatBuffers GRPC client/server test: completed successfully");
    }
}
