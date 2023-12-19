<?php
// automatically generated by the FlatBuffers compiler, do not modify

namespace MyGame;

use \Google\FlatBuffers\Struct;
use \Google\FlatBuffers\Table;
use \Google\FlatBuffers\ByteBuffer;
use \Google\FlatBuffers\FlatBufferBuilder;
use \Google\FlatBuffers\Constants;

class InParentNamespace extends Table
{
    /**
     * @param ByteBuffer $bb
     * @return InParentNamespace
     */
    public static function getRootAsInParentNamespace(ByteBuffer $bb)
    {
        $obj = new InParentNamespace();
        return ($obj->init($bb->getInt($bb->getPosition()) + $bb->getPosition(), $bb));
    }

    /**
     * @param ByteBuffer $bb
     * @return InParentNamespace
     */
    public static function getSizePrefixedRootAsInParentNamespace(ByteBuffer $bb)
    {
        $obj = new InParentNamespace();
        $bb->setPosition($bb->getPosition() + Constants::SIZEOF_INT);
        return ($obj->init($bb->getInt($bb->getPosition()) + $bb->getPosition(), $bb));
    }

    public static function InParentNamespaceIdentifier()
    {
        return "MONS";
    }

    public static function InParentNamespaceBufferHasIdentifier(ByteBuffer $buf)
    {
        return self::__has_identifier($buf, self::InParentNamespaceIdentifier());
    }

    public static function InParentNamespaceExtension()
    {
        return "mon";
    }

    /**
     * @param int $_i offset
     * @param ByteBuffer $_bb
     * @return InParentNamespace
     **/
    public function init($_i, ByteBuffer $_bb)
    {
        $this->bb_pos = $_i;
        $this->bb = $_bb;
        return $this;
    }

    /**
     * @param FlatBufferBuilder $builder
     * @return void
     */
    public static function startInParentNamespace(FlatBufferBuilder $builder)
    {
        $builder->StartObject(0);
    }

    /**
     * @param FlatBufferBuilder $builder
     * @return InParentNamespace
     */
    public static function createInParentNamespace(FlatBufferBuilder $builder, )
    {
        $builder->startObject(0);
        $o = $builder->endObject();
        return $o;
    }

    /**
     * @param FlatBufferBuilder $builder
     * @return int table offset
     */
    public static function endInParentNamespace(FlatBufferBuilder $builder)
    {
        $o = $builder->endObject();
        return $o;
    }
}
