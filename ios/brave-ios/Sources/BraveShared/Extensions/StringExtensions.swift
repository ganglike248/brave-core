// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

import Foundation
import SwiftUI
import UIKit

extension String {
  /// The first URL found within this String, or nil if no URL is found
  public var firstURL: URL? {
    if let detector = try? NSDataDetector(types: NSTextCheckingResult.CheckingType.link.rawValue),
      let match = detector.firstMatch(
        in: self,
        options: [],
        range: NSRange(location: 0, length: self.count)
      ),
      let range = Range(match.range, in: self)
    {
      return URL(string: String(self[range]))
    }
    return nil
  }

  /// Obtain a list of words in a given string
  public var words: [String] {
    var words: [String] = []
    enumerateSubstrings(
      in: startIndex..<endIndex,
      options: .byWords
    ) { (word, _, _, _) in
      if let word = word {
        words.append(word)
      }
    }
    return words
  }

  /// Encode a String to Base64
  public func toBase64() -> String {
    return Data(self.utf8).base64EncodedString()
  }

  /// Trim trailing and leading white space and new line characters to fetch better search suggestion text
  public var preferredSearchSuggestionText: String {
    return self.trimmingCharacters(in: CharacterSet.whitespacesAndNewlines)
  }
}

extension String {

  /// Image generation from String with changing font size
  /// - Parameter fontSize: Desired Font Size
  /// - Returns: Image generated from the String
  public func image(fontSize: CGFloat = 28.0) -> UIImage {
    let font = UIFont.systemFont(ofSize: fontSize)
    let size = self.size(withAttributes: [.font: font])

    return UIGraphicsImageRenderer(size: size).image { context in
      context.cgContext.setFillColor(UIColor.clear.cgColor)
      context.cgContext.setStrokeColor(UIColor.clear.cgColor)
      context.fill(CGRect(origin: .zero, size: size))

      (self as AnyObject).draw(in: CGRect(origin: .zero, size: size), withAttributes: [.font: font])
    }
  }

  public var regionFlagImage: UIImage? {
    // Root Unicode flags index
    let rootIndex: UInt32 = 127397
    var unicodeScalarView = ""

    for scalar in self.unicodeScalars {
      // Shift the letter index to the flags index
      if let appendedScalar = UnicodeScalar(rootIndex + scalar.value) {
        // Append symbol to the Unicode string
        unicodeScalarView.unicodeScalars.append(appendedScalar)
      }
    }

    if unicodeScalarView.isEmpty {
      return nil
    }

    return unicodeScalarView.image()
  }

  public var regionFlag: Image? {
    guard let flagImage = regionFlagImage else {
      return nil
    }

    return Image(uiImage: flagImage)
  }

  // Truncates the string in the middle to the specified maxLength characters.
  public func truncatingMiddle(maxLength: Int) -> String {
    guard maxLength > 1, count > maxLength else {
      return self
    }

    if maxLength == 2 {
      let start = self.prefix(1)
      let end = self.suffix(1)
      return "\(start)…\(end)"
    }

    let halfLength = (maxLength - 1) / 2
    let start = self.prefix(halfLength)
    let end = self.suffix(maxLength - halfLength - 1)
    return "\(start)…\(end)"
  }

  // Strips unicode control characters such as LTR, RTL, New-Lines, and illegal Characters
  public var strippingUnicodeControlCharacters: String {
    let validFilenameSet = CharacterSet(charactersIn: ":/")
      .union(.newlines)
      .union(.controlCharacters)
      .union(.illegalCharacters)
    return self.components(separatedBy: validFilenameSet).joined()
  }
}
