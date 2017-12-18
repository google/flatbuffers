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

package example;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;

import io.grpc.*;
import io.grpc.stub.StreamObserver;
import org.junit.Test;
import static org.junit.Assert.*;

import com.google.flatbuffers.FlatBufferBuilder;

public class MonsterGrpcTest {
	static Map<String, Monster> monsters = new HashMap<String, Monster>();
	static final String BIG_MONSTER_NAME = "big-monster";
	static {
		monsters.put(BIG_MONSTER_NAME, makeBigMonster());
	}
	static Monster makeBigMonster() {
		FlatBufferBuilder builder = new FlatBufferBuilder(0);

		// Create some weapons for our Monster ('Sword' and 'Axe').
		int weaponOneName = builder.createString("Sword");
		short weaponOneDamage = 3;
		int weaponTwoName = builder.createString("Axe");
		short weaponTwoDamage = 5;

		// Use the `createWeapon()` helper function to create the weapons, since we set every field.
		int[] weaps = new int[2];
		weaps[0] = Weapon.createWeapon(builder, weaponOneName, weaponOneDamage);
		weaps[1] = Weapon.createWeapon(builder, weaponTwoName, weaponTwoDamage);

		// Serialize the FlatBuffer data.
		int name = builder.createString("Orc");
		byte[] treasure = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
		int inv = Monster.createInventoryVector(builder, treasure);
		int weapons = Monster.createWeaponsVector(builder, weaps);
		int pos = Vec3.createVec3(builder, 1.0f, 2.0f, 3.0f);

		Monster.startMonster(builder);
		Monster.addPos(builder, pos);
		Monster.addName(builder, name);
		Monster.addColor(builder, Color.Red);
		Monster.addHp(builder, (short)300);
		Monster.addInventory(builder, inv);
		Monster.addWeapons(builder, weapons);
		Monster.addEquippedType(builder, Equipment.Weapon);
		Monster.addEquipped(builder, weaps[1]);
		int orc = Monster.endMonster(builder);

		builder.finish(orc); // You could also call `Monster.finishMonsterBuffer(builder, orc);`.

		// We now have a FlatBuffer that can be stored on disk or sent over a network.
		ByteBuffer buf = builder.dataBuffer();

		// Get access to the root:
		return Monster.getRootAsMonster(buf);
	}

	static class MyService extends MonsterSvcGrpc.MonsterSvcImplBase {
		@Override
		public void showMonster(MonsterRequest request, StreamObserver<Monster> responseObserver) {
			Monster monster = monsters.get(request.name());
			if (monster != null) {
				responseObserver.onNext(monster);
				responseObserver.onCompleted();
			} else {
				responseObserver.onError(Status.NOT_FOUND.asRuntimeException());
			}
		}
	};

	static int startServer() throws IOException {
		Server server = ServerBuilder.forPort(0).addService(new MyService()).build().start();
		return server.getPort();
	}


	@Test
	public void test() throws IOException {
		int port = startServer();
		ManagedChannel channel = ManagedChannelBuilder.forAddress("localhost", port)
				// Channels are secure by default (via SSL/TLS). For the example we disable TLS to avoid
				// needing certificates.
				.usePlaintext(true)
				.directExecutor()
				.build();

		MonsterSvcGrpc.MonsterSvcBlockingStub stub = MonsterSvcGrpc.newBlockingStub(channel);

		FlatBufferBuilder builder = new FlatBufferBuilder();
		int offsetStr1 = builder.createString(BIG_MONSTER_NAME);
		int offsetRequest = MonsterRequest.createMonsterRequest(builder, offsetStr1);
		builder.finish(offsetRequest);
		ByteBuffer buffer = builder.dataBuffer();
		MonsterRequest monsterRequest = MonsterRequest.getRootAsMonsterRequest(buffer);

		Monster monster = stub.showMonster(monsterRequest);

		// Note: We did not set the `mana` field explicitly, so we get back the default value.
	    assertEquals (monster.mana(), (short)150);
		assertEquals (monster.hp(), (short)300);
		assertEquals (monster.name(), "Orc");
		assertEquals (monster.color(), Color.Red);
		assertEquals (monster.pos().x(), 1.0f, 0);
		assertEquals (monster.pos().y(), 2.0f, 0);
		assertEquals (monster.pos().z(), 3.0f, 0);

	    // Get and test the `inventory` FlatBuffer `vector`.
	    for (int i = 0; i < monster.inventoryLength(); i++) {
	      assertEquals(monster.inventory(i), (byte)i);
	    }

	    // Get and test the `weapons` FlatBuffer `vector` of `table`s.
	    String[] expectedWeaponNames = {"Sword", "Axe"};
	    int[] expectedWeaponDamages = {3, 5};
	    for (int i = 0; i < monster.weaponsLength(); i++) {
	      assertEquals(monster.weapons(i).name(), expectedWeaponNames[i]);
	      assertEquals(monster.weapons(i).damage(), expectedWeaponDamages[i]);
	    }

	    // Get and test the `equipped` FlatBuffer `union`.
	    assertEquals(monster.equippedType(), Equipment.Weapon);
	    Weapon equipped = (Weapon)monster.equipped(new Weapon());
	    assertEquals(equipped.name(), "Axe");
	    assertEquals(equipped.damage(),5);

	    System.out.println("The FlatBuffer was successfully created, sent over the network, and verified!");
	}

}
