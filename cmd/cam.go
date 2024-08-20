package cmd

import (
	"fmt"

	"github.com/spf13/cobra"
)

var (
	camCmd = &cobra.Command{
		Use:   "cam [CONFIG]",
		Short: "Acquire images from a camera configured with CONFIG",
		Long:  "Acquire images from a camera configured with CONFIG",
		Args: cobra.MatchAll(
			cobra.ExactArgs(1),
		),
		Hidden: true,
		RunE:   cam,
	}
)

func cam(cmd *cobra.Command, args []string) error {
	return fmt.Errorf("%w: command enabled only on Linux", ErrCmdDisabled)
}
