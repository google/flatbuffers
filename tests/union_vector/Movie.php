<?php
// automatically generated by the FlatBuffers compiler, do not modify

declare(strict_types=1);

use \Google\FlatBuffers\Constants;
use \Google\FlatBuffers\Struct;
use \Google\FlatBuffers\Table;
use \Google\FlatBuffers\ByteBuffer;
use \Google\FlatBuffers\FlatbufferBuilder;

class Movie extends Table
{
    /**
     * @param ByteBuffer $bb
     * @return Movie
     */
    public static function getRootAsMovie(ByteBuffer $bb): Movie
    {
        $obj = new Movie();
        return $obj->init($bb->followUOffset($bb->getPosition()), $bb);
    }

    public static function MovieIdentifier(): string
    {
        return "MOVI";
    }

    public static function MovieBufferHasIdentifier(ByteBuffer $buf): bool
    {
        return self::__has_identifier($buf, self::MovieIdentifier());
    }

    /**
     * @param NPosT $_i offset
     * @param ByteBuffer $_bb
     * @return Movie
     **/
    public function init(int $_i, ByteBuffer $_bb): Movie
    {
        $this->bb_pos = $_i;
        $this->bb = $_bb;
        return $this;
    }

    /**
     * @return ByteT
     */
    public function getMainCharacterType(): int
    {
        $o = $this->__offset(4);
        return $o !== 0 ? $this->bb->getByte(Constants::asNPos($o + $this->bb_pos)) : \Character::NONE;
    }

    /**
     * @template T of Table|Struct
     *
     * @param T $obj
     * @return ?T
     */
    public function getMainCharacter(Table|Struct $obj): ?object
    {
        $o = $this->__offset(6);
        return $o !== 0 ? $this->__union($obj, $o) : null;
    }

    /**
     * @param UOffsetT $j offset
     * @return ?ByteT
     */
    public function getCharactersType(int $j): ?int
    {
        $o = $this->__offset(8);
        return $o !== 0 ? $this->bb->getByte(Constants::asNPos($this->__vector($o) + $j * 1)) : null;
    }

    /**
     * @return UOffsetT
     */
    public function getCharactersTypeLength(): int
    {
        $o = $this->__offset(8);
        return $o !== 0 ? $this->__vector_len($o) : 0;
    }

    /**
     * @param int offset
     * @return TableT
     */
    public function getCharacters($j, $obj)
    {
        $o = $this->__offset(10);
        return $o !== 0 ? $this->__union($obj, $this->__vector($o) + $j * 4 - $this->bb_pos) : null;
    }

    /**
     * @return UOffsetT
     */
    public function getCharactersLength(): int
    {
        $o = $this->__offset(10);
        return $o !== 0 ? $this->__vector_len($o) : 0;
    }

    /**
     * @param FlatbufferBuilder $builder
     */
    public static function startMovie(FlatbufferBuilder $builder): void
    {
        $builder->StartObject(4);
    }

    /**
     * @param FlatbufferBuilder $builder
     * @param ByteT $main_character_type
     * @param WPosT $main_character
     * @param WPosT $characters_type
     * @param WPosT $characters
     * @return WPosT
     */
    public static function createMovie(FlatbufferBuilder $builder, int $main_character_type, int $main_character, int $characters_type, int $characters): int
    {
        $builder->startObject(4);
        self::addMainCharacterType($builder, $main_character_type);
        self::addMainCharacter($builder, $main_character);
        self::addCharactersType($builder, $characters_type);
        self::addCharacters($builder, $characters);
        $o = $builder->endObject();
        return $o;
    }

    /**
     * @param FlatbufferBuilder $builder
     * @param ByteT $mainCharacterType
     */
    public static function addMainCharacterType(FlatbufferBuilder $builder, int $mainCharacterType): void
    {
        $builder->addByteX(0, $mainCharacterType, 0);
    }

    /**
     * @param FlatbufferBuilder $builder
     * @param WPosT $offset
     */
    public static function addMainCharacter(FlatbufferBuilder $builder, int $offset): void
    {
        $builder->addOffsetX(1, $offset, 0);
    }

    /**
     * @param FlatbufferBuilder $builder
     * @param WPosT $charactersType
     */
    public static function addCharactersType(FlatbufferBuilder $builder, int $charactersType): void
    {
        $builder->addOffsetX(2, $charactersType, 0);
    }

    /**
     * @param FlatbufferBuilder $builder
     * @param list<ByteT> $data data
     * @return WPosT vector offset
     */
    public static function createCharactersTypeVector(FlatbufferBuilder $builder, array $data): int
    {
        $builder->startVector(1, Constants::asUOffset(count($data)), 1);
        for ($i = count($data) - 1; $i >= 0; $i--) {
            $builder->putByte($data[$i]);
        }
        return $builder->endVector();
    }

    /**
     * @param FlatbufferBuilder $builder
     * @param UOffsetT $numElems
     */
    public static function startCharactersTypeVector(FlatbufferBuilder $builder, int $numElems): void
    {
        $builder->startVector(1, $numElems, 1);
    }

    /**
     * @param FlatbufferBuilder $builder
     * @param string $data byte string
     * @return WPosT vector offset
     */
    public static function createCharactersTypeVectorFromString(FlatbufferBuilder $builder, string $data): int
    {
        return $builder->createByteString($data);
    }

    /**
     * @param FlatbufferBuilder $builder
     * @param WPosT $characters
     */
    public static function addCharacters(FlatbufferBuilder $builder, int $characters): void
    {
        $builder->addOffsetX(3, $characters, 0);
    }

    /**
     * @param FlatbufferBuilder $builder
     * @param list<WPosT> $data data
     * @return WPosT vector offset
     */
    public static function createCharactersVector(FlatbufferBuilder $builder, array $data): int
    {
        $builder->startVector(4, Constants::asUOffset(count($data)), 4);
        for ($i = count($data) - 1; $i >= 0; $i--) {
            $builder->putOffset($data[$i]);
        }
        return $builder->endVector();
    }

    /**
     * @param FlatbufferBuilder $builder
     * @param UOffsetT $numElems
     */
    public static function startCharactersVector(FlatbufferBuilder $builder, int $numElems): void
    {
        $builder->startVector(4, $numElems, 4);
    }

    /**
     * @param FlatbufferBuilder $builder
     * @return WPosT table offset
     */
    public static function endMovie(FlatbufferBuilder $builder): int
    {
        $o = $builder->endObject();
        return $o;
    }

    /**
     * @param FlatbufferBuilder $builder
     * @param WPosT $offset
     */
    public static function finishMovieBuffer(FlatbufferBuilder $builder, int $offset): void
    {
        $builder->finish($offset, "MOVI");
    }
}
