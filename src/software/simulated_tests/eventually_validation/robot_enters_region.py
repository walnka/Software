import logging

import pytest

logging.basicConfig(
    level=logging.INFO,
    format=(
        "%(asctime)s - [%(levelname)s] - [%(threadName)s] -"
        "%(name)s - (%(filename)s).%(funcName)s(%(lineno)d) - %(message)s"
    ),
)
logger = logging.getLogger(__name__)


import software.geom.geometry as tbots_geom
from proto.geometry_pb2 import Angle, AngularVelocity, Point, Vector
from proto.tbots_software_msgs_pb2 import Vision
from proto.vision_pb2 import BallState, RobotState
from proto.world_pb2 import (
    SimulatorTick,
    ValidationGeometry,
    ValidationProto,
    ValidationStatus,
    World,
    WorldState,
)

from software.simulated_tests.validation import (
    EventuallyValidation,
    Validation,
    create_validation_geometry,
)


class RobotEntersRegion(Validation):

    """Checks if a Robot enters any of the provided regions."""

    def __init__(self, flipped = False, regions=[]):
        self.regions = regions
        self.flipped = flipped

    def _get_private_validation_status(self, vision) -> ValidationStatus:
        """Checks if _any_ robot enters the provided regions

        :param vision: The vision msg to validate
        :returns: FAILING until a robot enters any of the regions
                  PASSING when a robot enters
        """
        for region in self.regions:
            for robot_id, robot_states in vision.robot_states.items():
                if tbots_geom.contains(
                    region, tbots_geom.createPoint(robot_states.global_position)
                ):
                    return ValidationStatus.PASSING

        return ValidationStatus.FAILING

    def get_validation_geometry(self, vision) -> ValidationGeometry:
        """Returns the underlying geometry this validation is checking

        :param vision: The vision msg to validate
        :returns: ValidationGeometry containing geometry to visualize

        """
        return create_validation_geometry(self.regions)

    def get_failure_message(self):
        return "Robot didn't enter any of these regions: {}".format(self.regions)
