import assert from 'assert'
import * as flatbuffers from 'flatbuffers'

import { Character } from './union_vector/Character.js'
import { BookReader, BookReaderT } from './union_vector/BookReader.js'
import { Attacker, AttackerT } from './union_vector/Attacker.js'
import { Movie, MovieT } from './union_vector/Movie.js'

var charTypes = [
  Character.Belle,
  Character.MuLan,
  Character.BookFan,
  Character.Other
];

function testMovieBuf(movie) {
  assert.strictEqual(movie.characters_type_Length(), charTypes.length);
  assert.strictEqual(movie.characters_Length(), movie.characters_type_Length());

  for (var i = 0; i < charTypes.length; ++i) {
    assert.strictEqual(movie.characters_type(i), charTypes[i]);
  }

  var bookReader7 = movie.characters(0, new BookReader());
  assert.strictEqual(bookReader7.books_read(), 7);

  var attacker = movie.characters(1, new Attacker());
  assert.strictEqual(attacker.sword_attack_damage(), 5);

  var bookReader2 = movie.characters(2, new BookReader());
  assert.strictEqual(bookReader2.books_read(), 2);

  var other = movie.characters(3, '');
  assert.strictEqual(other, "I am other");
}

function testMovieUnpack(movie) {
  assert.strictEqual(movie.characters_type.length, charTypes.length);
  assert.strictEqual(movie.characters.length, movie.characters_type.length);

  for (var i = 0; i < charTypes.length; ++i) {
    assert.strictEqual(movie.characters_type[i], charTypes[i]);
  }

  var bookReader7 = movie.characters[0];
  assert.strictEqual(bookReader7 instanceof BookReaderT, true);
  assert.strictEqual(bookReader7.books_read, 7);

  var attacker = movie.characters[1];
  assert.strictEqual(attacker instanceof AttackerT, true);
  assert.strictEqual(attacker.sword_attack_damage, 5);

  var bookReader2 = movie.characters[2];
  assert.strictEqual(bookReader2 instanceof BookReaderT, true);
  assert.strictEqual(bookReader2.books_read, 2);

  var other = movie.characters[3];
  assert.strictEqual(other, "I am other");
}

function createMovie(fbb) {
  Attacker.startAttacker(fbb);
  Attacker.add_sword_attack_damage(fbb, 5);
  var attackerOffset = Attacker.endAttacker(fbb);

  var charTypesOffset = Movie.create_characters_type_Vector(fbb, charTypes);
  var charsOffset = 0;

  let otherOffset = fbb.createString("I am other");

  charsOffset = Movie.create_characters_Vector(
    fbb,
    [
      BookReader.createBookReader(fbb, 7),
      attackerOffset,
      BookReader.createBookReader(fbb, 2),
      otherOffset
    ]
  );

  Movie.startMovie(fbb);
  Movie.add_characters_type(fbb, charTypesOffset);
  Movie.add_characters(fbb, charsOffset);
  Movie.finishMovieBuffer(fbb, Movie.endMovie(fbb))
}

function main() {
  var fbb = new flatbuffers.Builder();

  createMovie(fbb);

  var buf = new flatbuffers.ByteBuffer(fbb.asUint8Array());

  var movie = Movie.getRootAsMovie(buf);
  testMovieBuf(movie);

  testMovieUnpack(movie.unpack());

  var movie_to = new MovieT();
  movie.unpackTo(movie_to);
  testMovieUnpack(movie_to);

  fbb.clear();
  Movie.finishMovieBuffer(fbb, movie_to.pack(fbb));
  var unpackBuf = new flatbuffers.ByteBuffer(fbb.asUint8Array());
  testMovieBuf(Movie.getRootAsMovie(unpackBuf));
  
  console.log('FlatBuffers union vector test: completed successfully');
}

main();
