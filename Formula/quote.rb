class Quote < Formula
  desc "Fast, real-time stock quotes in your terminal"
  homepage "https://github.com/Khurdhula-Harshavardhan/quote"
  url "https://github.com/Khurdhula-Harshavardhan/quote/archive/refs/tags/v0.0.3.tar.gz"
  sha256 "" # Will be calculated when you create a release
  license "MIT"
  head "https://github.com/Khurdhula-Harshavardhan/quote.git", branch: "main"

  depends_on "curl"

  def install
    system "make"
    bin.install "quote"
  end

  test do
    assert_match "Usage:", shell_output("#{bin}/quote --help")
  end
end
