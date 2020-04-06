var assert = require('assert');

var flatbuffers = require('../js/flatbuffers').flatbuffers;
var Test = require(process.argv[2]);

var isTsTest = !!process.env.FB_TS_TEST; 

var charTypes = [
  Test.Character.Belle,
  Test.Character.MuLan,
  Test.Character.BookFan,
];
if(isTsTest) { charTypes.push(Test.Character.Other); }

function testMovieBuf(movie) {
  assert.strictEqual(movie.charactersTypeLength(), charTypes.length);
  assert.strictEqual(movie.charactersLength(), movie.charactersTypeLength());

  for (var i = 0; i < charTypes.length; ++i) {
    assert.strictEqual(movie.charactersType(i), charTypes[i]);
  }

  var bookReader7 = movie.characters(0, new Test.BookReader());
  assert.strictEqual(bookReader7.booksRead(), 7);

  var attacker = movie.characters(1, new Test.Attacker());
  assert.strictEqual(attacker.swordAttackDamage(), 5);

  var bookReader2 = movie.characters(2, new Test.BookReader());
  assert.strictEqual(bookReader2.booksRead(), 2);

  if(isTsTest) {
    var other = movie.characters(3, '');
    assert.strictEqual(other, "I am other");
  }
}

function testMovieUnpack(movie) {
  assert.strictEqual(movie.charactersType.length, charTypes.length);
  assert.strictEqual(movie.characters.length, movie.charactersType.length);

  for (var i = 0; i < charTypes.length; ++i) {
    assert.strictEqual(movie.charactersType[i], charTypes[i]);
  }

  var bookReader7 = movie.characters[0];
  assert.strictEqual(bookReader7 instanceof Test.BookReaderT, true);
  assert.strictEqual(bookReader7.booksRead, 7);
  
  var attacker = movie.characters[1];
  assert.strictEqual(attacker instanceof Test.AttackerT, true);
  assert.strictEqual(attacker.swordAttackDamage, 5);
  
  var bookReader2 = movie.characters[2];
  assert.strictEqual(bookReader2 instanceof Test.BookReaderT, true);
  assert.strictEqual(bookReader2.booksRead, 2);

  if(isTsTest) {
    var other = movie.characters[3];
    assert.strictEqual(other, "I am other");
  }
}

function createMovie(fbb) {
  Test.Attacker.startAttacker(fbb);
  Test.Attacker.addSwordAttackDamage(fbb, 5);
  var attackerOffset = Test.Attacker.endAttacker(fbb);

  var charTypesOffset = Test.Movie.createCharactersTypeVector(fbb, charTypes);
  var charsOffset = 0;

  if(isTsTest) {
    let otherOffset = fbb.createString("I am other");

    charsOffset = Test.Movie.createCharactersVector(
      fbb,
      [
        Test.BookReader.createBookReader(fbb, 7),
        attackerOffset,
        Test.BookReader.createBookReader(fbb, 2),
        otherOffset
      ]
    );
  } else {
    charsOffset = Test.Movie.createCharactersVector(
      fbb,
      [
        Test.BookReader.createBookReader(fbb, 7),
        attackerOffset,
        Test.BookReader.createBookReader(fbb, 2)
      ]
    );
  }

  Test.Movie.startMovie(fbb);
  Test.Movie.addCharactersType(fbb, charTypesOffset);
  Test.Movie.addCharacters(fbb, charsOffset);
  Test.Movie.finishMovieBuffer(fbb, Test.Movie.endMovie(fbb))
}

function main() {
  var fbb = new flatbuffers.Builder();

  createMovie(fbb);

  var buf = new flatbuffers.ByteBuffer(fbb.asUint8Array());

  var movie = Test.Movie.getRootAsMovie(buf);
  testMovieBuf(movie);

  if(isTsTest) {
    testMovieUnpack(movie.unpack());

    var movie_to = new Test.MovieT();
    movie.unpackTo(movie_to);
    testMovieUnpack(movie_to);

    fbb.clear();
    Test.Movie.finishMovieBuffer(fbb, movie_to.pack(fbb));
    var unpackBuf = new flatbuffers.ByteBuffer(fbb.asUint8Array());
    testMovieBuf(Test.Movie.getRootAsMovie(unpackBuf));
  }

  console.log('FlatBuffers union vector test: completed successfully');
}

main();
