from __future__ import print_function

import os
import sys
import grpc
import flatbuffers

from concurrent import futures

sys.path.append(os.path.join(os.path.dirname(__file__), '..', '..', 'tests'))
import MyGame.Example.Monster as Monster
import MyGame.Example.Stat as Stat
import MyGame.Example.monster_test_grpc_fb as monster_grpc_fb

test_stat_id = "test_stat_id"
test_monster_name = "test_monster_name"


class MonsterStorage(monster_grpc_fb.MonsterStorageServicer):

    def Store(self, request, context):

        m = Monster.Monster().GetRootAsMonster(request, 0)

        b = flatbuffers.Builder(0)
        i = b.CreateString(test_stat_id)
        Stat.StatStart(b)
        Stat.StatAddId(b, i)
        b.Finish(Stat.StatEnd(b))
        return bytes(b.Output())

    def Retrieve(self, request, context):

        s = Stat.Stat().GetRootAsStat(request, 0)

        b = flatbuffers.Builder(0)
        i = b.CreateString(test_monster_name)
        Monster.MonsterStart(b)
        Monster.MonsterAddName(b, i)
        b.Finish(Monster.MonsterEnd(b))
        yield bytes(b.Output())


def serve():

    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    monster_grpc_fb.add_MonsterStorageServicer_to_server(MonsterStorage(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    run()


def run():

    channel = grpc.insecure_channel('127.0.0.1:50051')
    stub = monster_grpc_fb.MonsterStorageStub(channel)

    b = flatbuffers.Builder(0)
    i = b.CreateString(test_monster_name)
    Monster.MonsterStart(b)
    Monster.MonsterAddName(b, i)
    b.Finish(Monster.MonsterEnd(b))


    stat_response = stub.Store(bytes(b.Output()))
    s = Stat.Stat().GetRootAsStat(stat_response, 0)
    assert s.Id().decode("utf-8") == test_stat_id


    b = flatbuffers.Builder(0)
    i = b.CreateString(test_stat_id)
    Stat.StatStart(b)
    Stat.StatAddId(b, i)
    b.Finish(Stat.StatEnd(b))


    monster_reponses = stub.Retrieve(stat_response)
    for monster_reponse in monster_reponses:
        m = Monster.Monster().GetRootAsMonster(monster_reponse, 0)
        assert m.Name().decode("utf-8") == test_monster_name


if __name__ == '__main__':
  serve()
