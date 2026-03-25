import assert from 'assert'
import * as flatbuffers from 'flatbuffers'

import {Attacker, AttackerT} from './union_vector/attacker.js'
import {BookReader, BookReaderT} from './union_vector/book-reader.js'
import {Character} from './union_vector/character.js'
import {Rapunzel, RapunzelT} from './union_vector/rapunzel.js'
import {Movie, MovieT} from './union_vector/movie.js'

var charTypes =
    [Character.Belle, Character.MuLan, Character.BookFan, Character.Other];

function testMovieBuf(movie) {
  assert.strictEqual(movie.charactersTypeLength(), charTypes.length);
  assert.strictEqual(movie.charactersLength(), movie.charactersTypeLength());

  for (var i = 0; i < charTypes.length; ++i) {
    assert.strictEqual(movie.charactersType(i), charTypes[i]);
  }

  var bookReader7 = movie.characters(0, new BookReader());
  assert.strictEqual(bookReader7.booksRead(), 7);

  var attacker = movie.characters(1, new Attacker());
  assert.strictEqual(attacker.swordAttackDamage(), 5);

  var bookReader2 = movie.characters(2, new BookReader());
  assert.strictEqual(bookReader2.booksRead(), 2);

  var other = movie.characters(3, '');
  assert.strictEqual(other, 'I am other');
}

function testMovieUnpack(movie) {
  assert.strictEqual(movie.charactersType.length, charTypes.length);
  assert.strictEqual(movie.characters.length, movie.charactersType.length);

  for (var i = 0; i < charTypes.length; ++i) {
    assert.strictEqual(movie.charactersType[i], charTypes[i]);
  }

  var bookReader7 = movie.characters[0];
  assert.strictEqual(bookReader7 instanceof BookReaderT, true);
  assert.strictEqual(bookReader7.booksRead, 7);

  var attacker = movie.characters[1];
  assert.strictEqual(attacker instanceof AttackerT, true);
  assert.strictEqual(attacker.swordAttackDamage, 5);

  var bookReader2 = movie.characters[2];
  assert.strictEqual(bookReader2 instanceof BookReaderT, true);
  assert.strictEqual(bookReader2.booksRead, 2);

  var other = movie.characters[3];
  assert.strictEqual(other, 'I am other');
}

function createMovie(fbb) {
  Attacker.startAttacker(fbb);
  Attacker.addSwordAttackDamage(fbb, 5);
  var attackerOffset = Attacker.endAttacker(fbb);

  var charTypesOffset = Movie.createCharactersTypeVector(fbb, charTypes);
  var charsOffset = 0;

  let otherOffset = fbb.createString('I am other');

  charsOffset = Movie.createCharactersVector(fbb, [
    BookReader.createBookReader(fbb, 7), attackerOffset,
    BookReader.createBookReader(fbb, 2), otherOffset
  ]);

  Movie.startMovie(fbb);
  Movie.addCharactersType(fbb, charTypesOffset);
  Movie.addCharacters(fbb, charsOffset);
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

  testMovieClone();
  testMovieEquals();
  // Build a fresh buffer for unpackFields (the earlier fbb was reused).
  var fbb2 = new flatbuffers.Builder();
  createMovie(fbb2);
  testMovieUnpackFields(new flatbuffers.ByteBuffer(fbb2.asUint8Array()));

  console.log('FlatBuffers union vector test: completed successfully');
}

// Test clone() for union and vector-of-union fields.
function testMovieClone() {
  var mt = new MovieT();
  mt.mainCharacterType = Character.MuLan;
  mt.mainCharacter = new AttackerT(10);
  mt.charactersType = [Character.Belle, Character.Other, Character.Rapunzel];
  mt.characters = [new BookReaderT(5), 'hello', new RapunzelT(42)];

  var cloned = mt.clone();

  // Scalar union: cloned, not same reference
  assert.ok(cloned.mainCharacter instanceof AttackerT);
  assert.strictEqual(cloned.mainCharacter.swordAttackDamage, 10);

  // Vector-of-unions: types and values cloned
  assert.strictEqual(cloned.charactersType.length, 3);
  assert.strictEqual(cloned.charactersType[0], Character.Belle);
  assert.strictEqual(cloned.charactersType[1], Character.Other);
  assert.strictEqual(cloned.charactersType[2], Character.Rapunzel);
  assert.strictEqual(cloned.characters.length, 3);
  assert.ok(cloned.characters[0] instanceof BookReaderT);
  assert.strictEqual(cloned.characters[0].booksRead, 5);
  assert.strictEqual(cloned.characters[1], 'hello');
  assert.ok(cloned.characters[2] instanceof RapunzelT);
  assert.strictEqual(cloned.characters[2].hairLength, 42);

  // Vectors should be new references
  assert.notStrictEqual(cloned.characters, mt.characters);
  assert.notStrictEqual(cloned.charactersType, mt.charactersType);
}

// Test equals() for union fields.
function testMovieEquals() {
  var a = new MovieT();
  a.mainCharacterType = Character.MuLan;
  a.mainCharacter = new AttackerT(10);

  var b = new MovieT();
  b.mainCharacterType = Character.MuLan;
  b.mainCharacter = new AttackerT(10);

  assert.ok(a.equals(b));

  // Different union value
  b.mainCharacter = new AttackerT(99);
  assert.ok(!a.equals(b));

  // Different union type
  b.mainCharacterType = Character.Belle;
  b.mainCharacter = new BookReaderT(10);
  assert.ok(!a.equals(b));

  // Empty movies should be equal
  assert.ok(new MovieT().equals(new MovieT()));
}

// Test unpackFields with union and vector-of-union fields.
function testMovieUnpackFields(buf) {
  var movie = Movie.getRootAsMovie(buf);

  // Unpack only the vector-of-unions
  var partial = movie.unpackFields('characters_type', 'characters');
  assert.strictEqual(partial.charactersType.length, charTypes.length);
  assert.strictEqual(partial.characters.length, charTypes.length);
  assert.ok(partial.characters[0] instanceof BookReaderT);
  assert.strictEqual(partial.characters[0].booksRead, 7);
  assert.ok(partial.characters[1] instanceof AttackerT);
  assert.strictEqual(partial.characters[1].swordAttackDamage, 5);
  assert.strictEqual(partial.characters[3], 'I am other');

  // Unrequested field should be at default
  assert.strictEqual(partial.mainCharacterType, Character.NONE);
  assert.strictEqual(partial.mainCharacter, null);
}

main();
