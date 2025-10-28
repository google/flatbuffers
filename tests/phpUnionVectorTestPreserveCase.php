<?php

$phpLibDir = dirname(dirname(__FILE__));
require join(DIRECTORY_SEPARATOR, array($phpLibDir, "php", "Constants.php"));
require join(DIRECTORY_SEPARATOR, array($phpLibDir, "php", "ByteBuffer.php"));
require join(DIRECTORY_SEPARATOR, array($phpLibDir, "php", "FlatbufferBuilder.php"));
require join(DIRECTORY_SEPARATOR, array($phpLibDir, "php", "Table.php"));
require join(DIRECTORY_SEPARATOR, array($phpLibDir, "php", "Struct.php"));

$unionGeneratedDir = getenv('PHP_UNION_GENERATED_DIR');
if ($unionGeneratedDir === false || $unionGeneratedDir === '') {
    $unionGeneratedDir = join(DIRECTORY_SEPARATOR, array(dirname(__FILE__), "php"));
}
$unionGeneratedDir = rtrim($unionGeneratedDir, DIRECTORY_SEPARATOR);
foreach (['Attacker.php', 'BookReader.php', 'Character.php', 'Movie.php'] as $file) {
    $path = join(DIRECTORY_SEPARATOR, array($unionGeneratedDir, $file));
    if (!file_exists($path)) {
        throw new Exception("Required generated file not found: {$path}");
    }
    require $path;
}

class Assert {
    public function ok($result, $message = "") {
        if (!$result){
            throw new Exception(!empty($message) ? $message : "{$result} is not true.");
        }
    }

    public function Equal($result, $expected, $message = "") {
        if ($result != $expected) {
            throw new Exception(!empty($message) ? $message : "given the result {$result} is not equals as {$expected}");
        }
    }


    public function strictEqual($result, $expected, $message = "") {
        if ($result !== $expected) {
            throw new Exception(!empty($message) ? $message : "given the result {$result} is not strict equals as {$expected}");
        }
    }

    public function Throws($class, Callable $callback) {
        try {
            $callback();

            throw new \Exception("passed statement don't throw an exception.");
        } catch (\Exception $e) {
            if (get_class($e) != get_class($class)) {
                throw new Exception("passed statement doesn't throw " . get_class($class) . ". throwws " . get_class($e));
            }
        }
    }
}

function main()
{
    $assert = new Assert();

    $fbb = new Google\FlatBuffers\FlatBufferBuilder(1);

    $charTypes = [
        Character::Belle,
        Character::MuLan,
        Character::BookFan,
    ];

    Attacker::startAttacker($fbb);
    Attacker::addsword_attack_damage($fbb, 5);
    $attackerOffset = Attacker::endAttacker($fbb);

    $charTypesOffset = Movie::createcharacters_typeVector($fbb, $charTypes);
    $charsOffset = Movie::createcharactersVector(
        $fbb,
        [
            BookReader::createBookReader($fbb, 7),
            $attackerOffset,
            BookReader::createBookReader($fbb, 2),
        ]
    );

    Movie::startMovie($fbb);
    Movie::addcharacters_type($fbb, $charTypesOffset);
    Movie::addcharacters($fbb, $charsOffset);
    Movie::finishMovieBuffer($fbb, Movie::endMovie($fbb));

    $buf = Google\FlatBuffers\ByteBuffer::wrap($fbb->dataBuffer()->data());

    $movie = Movie::getRootAsMovie($buf);

    $assert->strictEqual($movie->getcharacters_typeLength(), count($charTypes));
    $assert->strictEqual($movie->getcharactersLength(), $movie->getcharacters_typeLength());

    for ($i = 0; $i < count($charTypes); ++$i) {
        $assert->strictEqual($movie->getcharacters_type($i), $charTypes[$i]);
    }

    $bookReader7 = $movie->getcharacters(0, new BookReader());
    $assert->strictEqual($bookReader7->Getbooks_read(), 7);

    $attacker = $movie->getcharacters(1, new Attacker());
    $assert->strictEqual($attacker->getsword_attack_damage(), 5);

    $bookReader2 = $movie->getcharacters(2, new BookReader());
    $assert->strictEqual($bookReader2->Getbooks_read(), 2);
}

try {
    main();
    exit(0);
} catch(Exception $e) {
    printf("Fatal error: Uncaught exception '%s' with message '%s. in %s:%d\n", get_class($e), $e->getMessage(), $e->getFile(), $e->getLine());
    printf("Stack trace:\n");
    echo $e->getTraceAsString() . PHP_EOL;
    printf("  thrown in in %s:%d\n", $e->getFile(), $e->getLine());

    die(-1);
}
