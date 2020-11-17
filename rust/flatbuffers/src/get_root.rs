/*
 * Copyright 2020 Google Inc. All rights reserved.
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

use crate::{
    Follow, ForwardsUOffset, InvalidFlatbuffer, SkipSizePrefix, Verifiable, Verifier,
    VerifierOptions,
};

/// Gets the root of the Flatbuffer, verifying it first with default options.
pub fn get_root<'buf, T>(data: &'buf [u8]) -> Result<T::Inner, InvalidFlatbuffer>
where
    T: 'buf + Follow<'buf> + Verifiable,
{
    let opts = VerifierOptions::default();
    get_root_opts::<T>(&opts, data)
}

#[inline]
/// Gets the root of the Flatbuffer, verifying it first with given options.
pub fn get_root_opts<'opts, 'buf, T>(
    opts: &'opts VerifierOptions,
    data: &'buf [u8],
) -> Result<T::Inner, InvalidFlatbuffer>
where
    T: 'buf + Follow<'buf> + Verifiable,
{
    let mut v = Verifier::new(&opts, data);
    <ForwardsUOffset<T>>::run_verifier(&mut v, 0)?;
    Ok(get_root_fast::<T>(data))
}

#[inline]
/// Gets the root of a size prefixed Flatbuffer, verifying it first with default options.
pub fn get_size_prefixed_root<'buf, T>(data: &'buf [u8]) -> Result<T::Inner, InvalidFlatbuffer>
where
    T: 'buf + Follow<'buf> + Verifiable,
{
    let opts = VerifierOptions::default();
    get_size_prefixed_root_opts::<T>(&opts, data)
}

#[inline]
/// Gets the root of a size prefixed Flatbuffer, verifying it first with given options.
pub fn get_size_prefixed_root_opts<'opts, 'buf, T>(
    opts: &'opts VerifierOptions,
    data: &'buf [u8],
) -> Result<T::Inner, InvalidFlatbuffer>
where
    T: 'buf + Follow<'buf> + Verifiable,
{
    let mut v = Verifier::new(&opts, data);
    <SkipSizePrefix<ForwardsUOffset<T>>>::run_verifier(&mut v, 0)?;
    Ok(get_size_prefixed_root_fast::<T>(data))
}

#[inline]
/// Gets root for a trusted Flatbuffer. No verification happens.
pub fn get_root_fast<'buf, T>(data: &'buf [u8]) -> T::Inner
where
    T: Follow<'buf> + 'buf,
{
    <ForwardsUOffset<T>>::follow(data, 0)
}

#[inline]
/// Gets root for a trusted, size prefixed, Flatbuffer. No verification happens.
pub fn get_size_prefixed_root_fast<'buf, T>(data: &'buf [u8]) -> T::Inner
where
    T: Follow<'buf> + 'buf,
{
    <SkipSizePrefix<ForwardsUOffset<T>>>::follow(data, 0)
}
