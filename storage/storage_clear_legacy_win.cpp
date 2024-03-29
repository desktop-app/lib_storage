// This file is part of Desktop App Toolkit,
// a set of libraries for developing nice desktop applications.
//
// For license and copyright information please follow this link:
// https://github.com/desktop-app/legal/blob/master/LEGAL
//
#include "storage/storage_clear_legacy.h"

#include <windows.h>

namespace Storage {
namespace details {

std::vector<QString> CollectFiles(
		const QString &base,
		size_type limit,
		const base::flat_set<QString> &skip) {
	Expects(base.endsWith('/'));
	Expects(limit > 0);

	const auto native = QDir::toNativeSeparators(base).toStdWString();
	const auto search = native + L'*';

	auto data = WIN32_FIND_DATA{ 0 };
	const auto handle = FindFirstFileEx(
		search.c_str(),
		FindExInfoBasic,
		&data,
		FindExSearchNameMatch,
		nullptr,
		0);
	if (handle == INVALID_HANDLE_VALUE) {
		return {};
	}
	const auto guard = gsl::finally([&] { FindClose(handle); });

	auto result = std::vector<QString>();
	do {
		const auto full = native + data.cFileName;
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}
		const auto file = QString::fromWCharArray(
			data.cFileName,
			full.size() - native.size());
		auto name = QDir::fromNativeSeparators(file);
		if (!skip.contains(name)) {
			result.push_back(std::move(name));
		}
	} while (result.size() != limit && FindNextFile(handle, &data));

	return result;
}

bool RemoveLegacyFile(const QString &path) {
	const auto native = QDir::toNativeSeparators(path).toStdWString();
	return (::DeleteFile(native.c_str()) != 0);
}

} // namespace details
} // namespace Storage
