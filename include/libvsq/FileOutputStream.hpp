/**
 * @file FileOutputStream.hpp
 * Copyright © 2012,2014 kbinani
 *
 * This file is part of libvsq.
 *
 * libvsq is free software; you can redistribute it and/or
 * modify it under the terms of the BSD License.
 *
 * libvsq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#pragma once

#include "OutputStream.hpp"
#include <string>
#include <fstream>

LIBVSQ_BEGIN_NAMESPACE

/**
 * @brief ファイルへの出力を行う, OutputStream の実装.
 */
class FileOutputStream : public OutputStream
{
private:
	/**
	 * @brief 出力先のストリーム.
	 */
	std::ofstream stream;

public:
	/**
	 * @brief 出力先のファイルパスを指定し, 初期化する.
	 */
	explicit FileOutputStream(std::string const& filePath);

	~FileOutputStream();

	void write(char const* buffer, int64_t startIndex, int64_t length) override;

	void write(int value) override;

	void seek(int64_t position) override;

	int64_t getPointer() override;

	void close() override;
};

LIBVSQ_END_NAMESPACE
