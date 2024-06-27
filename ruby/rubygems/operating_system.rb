module Gem
  class << self

    ##
    # Detects --build-root option specified on command line.

    def opt_build_root?
      @opt_build_root ||= ARGV.include?('--build-root')
    end
    private :opt_build_root?

    ##
    # Regular user installs into user directory, except when --build-root is
    # specified during packaging.
    # TODO: check if we want to set --bindir to .local/bin

    remove_method :operating_system_defaults
    def operating_system_defaults
      unless opt_build_root?
        {'gem' => '--user-install'}
      else
        {}
      end
    end

    ##
    # Avoid duplicating install extensions in legacy location

    remove_method :install_extension_in_lib
    def install_extension_in_lib
      false
    end

  end
end
